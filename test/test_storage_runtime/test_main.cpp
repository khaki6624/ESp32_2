#include <Arduino.h>
#include <StorageRuntime.h>
#include <string.h>
#include <type_traits>
#include <unity.h>

namespace
{
constexpr size_t UPDATE_SEQUENCE_CAPACITY = 8U;

void assertResult(StorageResult expected, StorageResult actual)
{ TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected), static_cast<uint8_t>(actual)); }
void assertState(StorageOperationState expected, StorageOperationState actual)
{ TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected), static_cast<uint8_t>(actual)); }

StorageKey makeKey(const char* value = "config/runtime")
{
    StorageKey result;
    TEST_ASSERT_TRUE(result.set(value));
    return result;
}

class FakeStorageBackend final : public StorageBackend
{
public:
    StorageBackendResult beginResult = StorageBackendResult::SUCCESS;
    StorageBackendResult startResult = StorageBackendResult::ACCEPTED;
    size_t startTransferredLength = 0U;
    StorageBackendResult cancelResult = StorageBackendResult::CANCELLED;
    bool ready = true;
    size_t beginCalls = 0U;
    size_t readyCalls = 0U;
    size_t startCalls = 0U;
    size_t updateCalls = 0U;
    size_t cancelCalls = 0U;
    StorageRequest lastRequest{};
    StorageBackendResult updateResults[UPDATE_SEQUENCE_CAPACITY] = {};
    size_t updateLengths[UPDATE_SEQUENCE_CAPACITY] = {};
    size_t updateCount = 0U;
    size_t updateIndex = 0U;

    StorageBackendResult begin() override { ++beginCalls; return beginResult; }
    bool isReady() const override
    {
        ++const_cast<FakeStorageBackend*>(this)->readyCalls;
        return ready;
    }
    StorageBackendResult start(const StorageRequest& request,
        size_t& transferredLength) override
    {
        ++startCalls;
        lastRequest = request;
        transferredLength = startTransferredLength;
        return startResult;
    }
    StorageBackendResult update(size_t& transferredLength) override
    {
        ++updateCalls;
        if (updateIndex >= updateCount) return StorageBackendResult::FAILED;
        transferredLength = updateLengths[updateIndex];
        return updateResults[updateIndex++];
    }
    StorageBackendResult cancel() override { ++cancelCalls; return cancelResult; }

    void addUpdate(StorageBackendResult result, size_t transferredLength)
    {
        TEST_ASSERT_LESS_THAN_UINT32(UPDATE_SEQUENCE_CAPACITY, updateCount);
        updateResults[updateCount] = result;
        updateLengths[updateCount] = transferredLength;
        ++updateCount;
    }
};
}

void test_storage_key_policy_boundaries_and_atomic_failure()
{
    StorageKey key;
    TEST_ASSERT_FALSE(key.isValid());
    TEST_ASSERT_FALSE(key.set(nullptr));
    TEST_ASSERT_FALSE(key.set(""));
    TEST_ASSERT_TRUE(key.set("device/12.State_value-test"));
    TEST_ASSERT_EQUAL_STRING("device/12.State_value-test", key.c_str());
    const char* invalid[] = {"config runtime", "config\\runtime", "key=value",
        "key|value", "key:value", "key\rvalue", "key\nvalue", "key\tvalue"};
    for (size_t index = 0U; index < sizeof(invalid) / sizeof(invalid[0]); ++index)
    {
        TEST_ASSERT_FALSE(key.set(invalid[index]));
        TEST_ASSERT_EQUAL_STRING("device/12.State_value-test", key.c_str());
    }
    char maximum[STORAGE_KEY_MAX_LENGTH] = {};
    memset(maximum, 'a', sizeof(maximum) - 1U);
    TEST_ASSERT_TRUE(key.set(maximum));
    TEST_ASSERT_EQUAL_UINT32(STORAGE_KEY_MAX_LENGTH - 1U, key.length());
    char unterminated[STORAGE_KEY_MAX_LENGTH]; memset(unterminated, 'a', sizeof(unterminated));
    TEST_ASSERT_FALSE(key.set(unterminated));
    TEST_ASSERT_EQUAL_UINT32(STORAGE_KEY_MAX_LENGTH - 1U, key.length());
    StorageKey same; TEST_ASSERT_TRUE(same.set(maximum));
    TEST_ASSERT_TRUE(key.equals(same));
    TEST_ASSERT_FALSE(key.equals(makeKey("other/key")));
}

void test_buffer_views_are_bounded_non_owning_and_binary_safe()
{
    uint8_t binary[] = {0x01U, 0x00U, 0xFFU, '|', '\n', 0xD8U, 0xAFU};
    StorageReadBuffer input(binary, sizeof(binary));
    StorageWriteBuffer output(binary, sizeof(binary));
    TEST_ASSERT_TRUE(input.isValid()); TEST_ASSERT_TRUE(output.isValid());
    TEST_ASSERT_EQUAL_PTR(binary, input.data()); TEST_ASSERT_EQUAL_PTR(binary, output.data());
    TEST_ASSERT_EQUAL_UINT32(sizeof(binary), input.length());
    TEST_ASSERT_EQUAL_UINT32(sizeof(binary), output.capacity());
    TEST_ASSERT_FALSE(StorageReadBuffer(nullptr, 1U).isValid());
    TEST_ASSERT_FALSE(StorageReadBuffer(binary, 0U).isValid());
    TEST_ASSERT_FALSE(StorageReadBuffer(binary, STORAGE_MAX_DATA_LENGTH + 1U).isValid());
    TEST_ASSERT_FALSE(StorageWriteBuffer(nullptr, 1U).isValid());
    TEST_ASSERT_FALSE(StorageWriteBuffer(binary, 0U).isValid());
    TEST_ASSERT_FALSE(StorageWriteBuffer(binary, STORAGE_MAX_DATA_LENGTH + 1U).isValid());
    TEST_ASSERT_EQUAL_HEX8(0x00U, input.data()[1]);
}

void test_request_factories_enforce_unambiguous_operation_contracts()
{
    StorageKey key = makeKey();
    uint8_t data[] = {1U, 0U, 255U};
    StorageRequest read = StorageRequest::read(1U, key, StorageWriteBuffer(data, sizeof(data)));
    StorageRequest write = StorageRequest::write(2U, key, StorageReadBuffer(data, sizeof(data)), true);
    StorageRequest remove = StorageRequest::remove(3U, key);
    StorageRequest exists = StorageRequest::exists(4U, key);
    TEST_ASSERT_TRUE(read.isValid()); TEST_ASSERT_TRUE(write.isValid());
    TEST_ASSERT_TRUE(remove.isValid()); TEST_ASSERT_TRUE(exists.isValid());
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(StorageOperationType::READ), static_cast<uint8_t>(read.type()));
    TEST_ASSERT_TRUE(write.overwrite()); TEST_ASSERT_FALSE(read.overwrite());
    TEST_ASSERT_EQUAL_PTR(data, write.input().data()); TEST_ASSERT_EQUAL_UINT32(sizeof(data), write.input().length());
    TEST_ASSERT_FALSE(read.input().isValid()); TEST_ASSERT_FALSE(write.output().isValid());
    TEST_ASSERT_FALSE(remove.input().isValid()); TEST_ASSERT_FALSE(remove.output().isValid());
    TEST_ASSERT_FALSE(StorageRequest{}.isValid());
    TEST_ASSERT_FALSE(StorageRequest::read(0U, key, StorageWriteBuffer(data, sizeof(data))).isValid());
    TEST_ASSERT_FALSE(StorageRequest::write(1U, StorageKey{}, StorageReadBuffer(data, sizeof(data)), false).isValid());
    TEST_ASSERT_FALSE(StorageRequest::read(1U, key, StorageWriteBuffer{}).isValid());
    TEST_ASSERT_FALSE(StorageRequest::write(1U, key, StorageReadBuffer{}, false).isValid());
}

void test_begin_maps_results_calls_once_and_clears_previous_state()
{
    FakeStorageBackend backend; StorageRuntime runtime(backend);
    TEST_ASSERT_FALSE(runtime.isInitialized()); TEST_ASSERT_FALSE(runtime.isBusy());
    backend.beginResult = StorageBackendResult::FAILED;
    assertResult(StorageResult::BACKEND_FAILED, runtime.begin());
    TEST_ASSERT_FALSE(runtime.isInitialized()); TEST_ASSERT_EQUAL_UINT32(1U, backend.beginCalls);
    backend.beginResult = StorageBackendResult::RETRY_LATER;
    assertResult(StorageResult::BACKEND_RETRY_LATER, runtime.begin());
    backend.beginResult = StorageBackendResult::IN_PROGRESS;
    assertResult(StorageResult::BACKEND_NOT_READY, runtime.begin());
    backend.beginResult = StorageBackendResult::SUCCESS;
    assertResult(StorageResult::SUCCESS, runtime.begin());
    TEST_ASSERT_TRUE(runtime.isInitialized()); TEST_ASSERT_EQUAL_UINT32(4U, backend.beginCalls);

    uint8_t data[] = {1U};
    assertResult(StorageResult::ACCEPTED, runtime.submit(StorageRequest::write(
        9U, makeKey(), StorageReadBuffer(data, sizeof(data)), true)));
    TEST_ASSERT_TRUE(runtime.isBusy());
    const size_t beginCallsBeforeBusyBegin = backend.beginCalls;
    const StorageOperationId activeId = runtime.transaction().operationId();
    assertResult(StorageResult::BUSY, runtime.begin());
    TEST_ASSERT_EQUAL_UINT32(beginCallsBeforeBusyBegin, backend.beginCalls);
    TEST_ASSERT_TRUE(runtime.isInitialized()); TEST_ASSERT_TRUE(runtime.isBusy());
    TEST_ASSERT_EQUAL_UINT32(activeId, runtime.transaction().operationId());
    assertState(StorageOperationState::PENDING, runtime.transaction().state());
    backend.cancelResult = StorageBackendResult::SUCCESS;
    assertResult(StorageResult::CANCELLED, runtime.cancel());
    assertResult(StorageResult::SUCCESS, runtime.begin());
    TEST_ASSERT_EQUAL_UINT32(beginCallsBeforeBusyBegin + 1U, backend.beginCalls);
    TEST_ASSERT_FALSE(runtime.isBusy()); TEST_ASSERT_FALSE(runtime.transaction().isValid());
}

void test_submit_mapping_atomic_failures_and_busy_policy()
{
    FakeStorageBackend backend; StorageRuntime runtime(backend);
    uint8_t firstData[] = {1U, 0U, 255U};
    uint8_t secondData[] = {2U};
    StorageRequest first = StorageRequest::write(10U, makeKey(),
        StorageReadBuffer(firstData, sizeof(firstData)), true);
    StorageRequest second = StorageRequest::write(11U, makeKey("other/key"),
        StorageReadBuffer(secondData, sizeof(secondData)), false);
    assertResult(StorageResult::NOT_INITIALIZED, runtime.submit(first));
    runtime.begin();
    assertResult(StorageResult::INVALID_ARGUMENT, runtime.submit(StorageRequest{}));
    backend.ready = false;
    assertResult(StorageResult::BACKEND_NOT_READY, runtime.submit(first));
    TEST_ASSERT_EQUAL_UINT32(0U, backend.startCalls);
    backend.ready = true;
    backend.startResult = StorageBackendResult::ACCEPTED;
    assertResult(StorageResult::ACCEPTED, runtime.submit(first));
    TEST_ASSERT_TRUE(runtime.isBusy()); TEST_ASSERT_EQUAL_UINT32(1U, backend.startCalls);
    TEST_ASSERT_EQUAL_UINT32(10U, runtime.transaction().operationId());
    assertState(StorageOperationState::PENDING, runtime.transaction().state());
    TEST_ASSERT_EQUAL_PTR(firstData, backend.lastRequest.input().data());
    assertResult(StorageResult::BUSY, runtime.submit(second));
    TEST_ASSERT_EQUAL_UINT32(1U, backend.startCalls);
    TEST_ASSERT_EQUAL_UINT32(10U, runtime.transaction().operationId());

    backend.cancelResult = StorageBackendResult::SUCCESS; runtime.cancel();
    const StorageBackendResult backendResults[] = {StorageBackendResult::IN_PROGRESS,
        StorageBackendResult::SUCCESS, StorageBackendResult::RETRY_LATER,
        StorageBackendResult::NOT_FOUND, StorageBackendResult::ALREADY_EXISTS,
        StorageBackendResult::BUFFER_TOO_SMALL, StorageBackendResult::FAILED};
    const StorageResult expected[] = {StorageResult::ACCEPTED, StorageResult::SUCCESS,
        StorageResult::BACKEND_RETRY_LATER, StorageResult::NOT_FOUND,
        StorageResult::ALREADY_EXISTS, StorageResult::BUFFER_TOO_SMALL,
        StorageResult::BACKEND_FAILED};
    for (size_t index = 0U; index < sizeof(expected) / sizeof(expected[0]); ++index)
    {
        backend.startResult = backendResults[index];
        assertResult(expected[index], runtime.submit(second));
        if (runtime.isBusy()) { backend.cancelResult = StorageBackendResult::SUCCESS; runtime.cancel(); }
        else TEST_ASSERT_TRUE(runtime.transaction().isValid());
    }
}

void test_update_integration_sequence_is_non_blocking_and_cumulative()
{
    FakeStorageBackend backend; StorageRuntime runtime(backend); runtime.begin();
    uint8_t data[] = {0x01U, 0x00U, 0xFFU};
    StorageRequest request = StorageRequest::write(20U, makeKey(),
        StorageReadBuffer(data, sizeof(data)), true);
    backend.addUpdate(StorageBackendResult::IN_PROGRESS, 1U);
    backend.addUpdate(StorageBackendResult::RETRY_LATER, 1U);
    backend.addUpdate(StorageBackendResult::IN_PROGRESS, 2U);
    backend.addUpdate(StorageBackendResult::SUCCESS, 3U);
    assertResult(StorageResult::ACCEPTED, runtime.submit(request));
    assertResult(StorageResult::IN_PROGRESS, runtime.update());
    TEST_ASSERT_EQUAL_UINT32(1U, runtime.transaction().transferredLength());
    assertResult(StorageResult::BACKEND_RETRY_LATER, runtime.update());
    TEST_ASSERT_TRUE(runtime.isBusy()); TEST_ASSERT_EQUAL_UINT32(1U, runtime.transaction().transferredLength());
    assertResult(StorageResult::IN_PROGRESS, runtime.update());
    TEST_ASSERT_EQUAL_UINT32(2U, runtime.transaction().transferredLength());
    assertResult(StorageResult::SUCCESS, runtime.update());
    TEST_ASSERT_FALSE(runtime.isBusy()); TEST_ASSERT_EQUAL_UINT32(4U, backend.updateCalls);
    TEST_ASSERT_EQUAL_UINT32(3U, runtime.transaction().transferredLength());
    assertState(StorageOperationState::SUCCEEDED, runtime.transaction().state());
    TEST_ASSERT_EQUAL_MEMORY(data, backend.lastRequest.input().data(), sizeof(data));
}

void test_start_transferred_length_contract_for_all_results_and_operations()
{
    uint8_t readData[32] = {};
    uint8_t writeData[8] = {};
    const StorageKey key = makeKey();

    FakeStorageBackend readSuccessBackend;
    StorageRuntime readSuccess(readSuccessBackend); readSuccess.begin();
    readSuccessBackend.startResult = StorageBackendResult::SUCCESS;
    readSuccessBackend.startTransferredLength = 12U;
    assertResult(StorageResult::SUCCESS, readSuccess.submit(StorageRequest::read(
        70U, key, StorageWriteBuffer(readData, sizeof(readData)))));
    TEST_ASSERT_FALSE(readSuccess.isBusy());
    assertState(StorageOperationState::SUCCEEDED, readSuccess.transaction().state());
    TEST_ASSERT_EQUAL_UINT32(12U, readSuccess.transaction().transferredLength());

    FakeStorageBackend writeSuccessBackend;
    StorageRuntime writeSuccess(writeSuccessBackend); writeSuccess.begin();
    writeSuccessBackend.startResult = StorageBackendResult::SUCCESS;
    writeSuccessBackend.startTransferredLength = sizeof(writeData);
    assertResult(StorageResult::SUCCESS, writeSuccess.submit(StorageRequest::write(
        71U, key, StorageReadBuffer(writeData, sizeof(writeData)), true)));
    TEST_ASSERT_EQUAL_UINT32(sizeof(writeData), writeSuccess.transaction().transferredLength());

    FakeStorageBackend acceptedBackend;
    StorageRuntime accepted(acceptedBackend); accepted.begin();
    acceptedBackend.startTransferredLength = 1U;
    assertResult(StorageResult::ACCEPTED, accepted.submit(StorageRequest::write(
        72U, key, StorageReadBuffer(writeData, sizeof(writeData)), false)));
    TEST_ASSERT_TRUE(accepted.isBusy());
    assertState(StorageOperationState::PENDING, accepted.transaction().state());
    TEST_ASSERT_EQUAL_UINT32(1U, accepted.transaction().transferredLength());

    FakeStorageBackend progressBackend;
    StorageRuntime progress(progressBackend); progress.begin();
    progressBackend.startResult = StorageBackendResult::IN_PROGRESS;
    progressBackend.startTransferredLength = 2U;
    progressBackend.addUpdate(StorageBackendResult::IN_PROGRESS, 1U);
    assertResult(StorageResult::ACCEPTED, progress.submit(StorageRequest::write(
        73U, key, StorageReadBuffer(writeData, sizeof(writeData)), false)));
    TEST_ASSERT_TRUE(progress.isBusy());
    assertState(StorageOperationState::RUNNING, progress.transaction().state());
    TEST_ASSERT_EQUAL_UINT32(2U, progress.transaction().transferredLength());
    assertResult(StorageResult::INTERNAL_ERROR, progress.update());
    TEST_ASSERT_FALSE(progress.isBusy());
    assertState(StorageOperationState::FAILED, progress.transaction().state());

    FakeStorageBackend oversizedReadBackend;
    StorageRuntime oversizedRead(oversizedReadBackend); oversizedRead.begin();
    oversizedReadBackend.startResult = StorageBackendResult::SUCCESS;
    oversizedReadBackend.startTransferredLength = sizeof(readData) + 1U;
    assertResult(StorageResult::INTERNAL_ERROR, oversizedRead.submit(StorageRequest::read(
        74U, key, StorageWriteBuffer(readData, sizeof(readData)))));
    TEST_ASSERT_FALSE(oversizedRead.isBusy());
    assertState(StorageOperationState::FAILED, oversizedRead.transaction().state());

    FakeStorageBackend oversizedWriteBackend;
    StorageRuntime oversizedWrite(oversizedWriteBackend); oversizedWrite.begin();
    oversizedWriteBackend.startResult = StorageBackendResult::SUCCESS;
    oversizedWriteBackend.startTransferredLength = sizeof(writeData) + 1U;
    assertResult(StorageResult::INTERNAL_ERROR, oversizedWrite.submit(StorageRequest::write(
        75U, key, StorageReadBuffer(writeData, sizeof(writeData)), false)));

    FakeStorageBackend removeBackend;
    StorageRuntime removeRuntime(removeBackend); removeRuntime.begin();
    removeBackend.startResult = StorageBackendResult::SUCCESS;
    removeBackend.startTransferredLength = 1U;
    assertResult(StorageResult::INTERNAL_ERROR,
        removeRuntime.submit(StorageRequest::remove(76U, key)));

    FakeStorageBackend existsBackend;
    StorageRuntime existsRuntime(existsBackend); existsRuntime.begin();
    existsBackend.startResult = StorageBackendResult::SUCCESS;
    existsBackend.startTransferredLength = 1U;
    assertResult(StorageResult::INTERNAL_ERROR,
        existsRuntime.submit(StorageRequest::exists(77U, key)));

    FakeStorageBackend retryBackend;
    StorageRuntime retryRuntime(retryBackend); retryRuntime.begin();
    retryBackend.startResult = StorageBackendResult::RETRY_LATER;
    assertResult(StorageResult::BACKEND_RETRY_LATER, retryRuntime.submit(
        StorageRequest::write(78U, key, StorageReadBuffer(writeData, sizeof(writeData)), false)));
    TEST_ASSERT_FALSE(retryRuntime.isBusy());
    assertState(StorageOperationState::FAILED, retryRuntime.transaction().state());
    TEST_ASSERT_EQUAL_UINT32(78U, retryRuntime.transaction().operationId());

    FakeStorageBackend invalidRetryBackend;
    StorageRuntime invalidRetry(invalidRetryBackend); invalidRetry.begin();
    invalidRetryBackend.startResult = StorageBackendResult::RETRY_LATER;
    invalidRetryBackend.startTransferredLength = 1U;
    assertResult(StorageResult::INTERNAL_ERROR, invalidRetry.submit(
        StorageRequest::write(79U, key, StorageReadBuffer(writeData, sizeof(writeData)), false)));
    TEST_ASSERT_FALSE(invalidRetry.isBusy());

    FakeStorageBackend failedBackend;
    StorageRuntime failed(failedBackend); failed.begin();
    failedBackend.startResult = StorageBackendResult::FAILED;
    failedBackend.startTransferredLength = 1U;
    assertResult(StorageResult::INTERNAL_ERROR, failed.submit(
        StorageRequest::write(80U, key, StorageReadBuffer(writeData, sizeof(writeData)), false)));
    TEST_ASSERT_EQUAL_UINT32(80U, failed.transaction().operationId());

    FakeStorageBackend notFoundBackend;
    StorageRuntime notFound(notFoundBackend); notFound.begin();
    notFoundBackend.startResult = StorageBackendResult::NOT_FOUND;
    notFoundBackend.startTransferredLength = 1U;
    assertResult(StorageResult::INTERNAL_ERROR, notFound.submit(
        StorageRequest::read(81U, key, StorageWriteBuffer(readData, sizeof(readData)))));
    TEST_ASSERT_EQUAL_UINT32(81U, notFound.transaction().operationId());
}

void test_update_maps_terminal_results_and_rejects_invalid_progress()
{
    uint8_t data[8] = {};
    const StorageBackendResult terminal[] = {StorageBackendResult::NOT_FOUND,
        StorageBackendResult::BUFFER_TOO_SMALL, StorageBackendResult::FAILED,
        StorageBackendResult::CANCELLED, static_cast<StorageBackendResult>(255U)};
    const StorageResult expected[] = {StorageResult::NOT_FOUND, StorageResult::BUFFER_TOO_SMALL,
        StorageResult::BACKEND_FAILED, StorageResult::CANCELLED, StorageResult::INTERNAL_ERROR};
    for (size_t index = 0U; index < sizeof(expected) / sizeof(expected[0]); ++index)
    {
        FakeStorageBackend backend; StorageRuntime runtime(backend); runtime.begin();
        backend.addUpdate(terminal[index], 0U);
        runtime.submit(StorageRequest::write(static_cast<StorageOperationId>(30U + index),
            makeKey(), StorageReadBuffer(data, sizeof(data)), false));
        assertResult(expected[index], runtime.update());
        TEST_ASSERT_FALSE(runtime.isBusy());
        TEST_ASSERT_EQUAL_UINT32(30U + index, runtime.transaction().operationId());
    }

    FakeStorageBackend decreasingBackend; StorageRuntime decreasing(decreasingBackend); decreasing.begin();
    decreasingBackend.addUpdate(StorageBackendResult::IN_PROGRESS, 2U);
    decreasingBackend.addUpdate(StorageBackendResult::IN_PROGRESS, 1U);
    decreasing.submit(StorageRequest::write(40U, makeKey(), StorageReadBuffer(data, sizeof(data)), false));
    assertResult(StorageResult::IN_PROGRESS, decreasing.update());
    assertResult(StorageResult::INTERNAL_ERROR, decreasing.update());
    TEST_ASSERT_FALSE(decreasing.isBusy());

    FakeStorageBackend oversized; StorageRuntime oversizedRuntime(oversized); oversizedRuntime.begin();
    oversized.addUpdate(StorageBackendResult::SUCCESS, sizeof(data) + 1U);
    oversizedRuntime.submit(StorageRequest::write(41U, makeKey(), StorageReadBuffer(data, sizeof(data)), false));
    assertResult(StorageResult::INTERNAL_ERROR, oversizedRuntime.update());

    uint8_t output[4] = {};
    FakeStorageBackend readBackend; StorageRuntime readRuntime(readBackend); readRuntime.begin();
    readBackend.startResult = StorageBackendResult::IN_PROGRESS;
    readBackend.addUpdate(StorageBackendResult::SUCCESS, sizeof(output) + 1U);
    readRuntime.submit(StorageRequest::read(42U, makeKey(), StorageWriteBuffer(output, sizeof(output))));
    assertResult(StorageResult::INTERNAL_ERROR, readRuntime.update());

    FakeStorageBackend removeBackend; StorageRuntime removeRuntime(removeBackend); removeRuntime.begin();
    removeBackend.addUpdate(StorageBackendResult::SUCCESS, 1U);
    removeRuntime.submit(StorageRequest::remove(43U, makeKey()));
    assertResult(StorageResult::INTERNAL_ERROR, removeRuntime.update());
    FakeStorageBackend existsBackend; StorageRuntime existsRuntime(existsBackend); existsRuntime.begin();
    existsBackend.addUpdate(StorageBackendResult::SUCCESS, 1U);
    existsRuntime.submit(StorageRequest::exists(44U, makeKey()));
    assertResult(StorageResult::INTERNAL_ERROR, existsRuntime.update());
}

void test_cancel_and_clear_completed_policies_preserve_metadata()
{
    FakeStorageBackend backend; StorageRuntime runtime(backend); uint8_t data[] = {1U};
    StorageRequest request = StorageRequest::write(50U, makeKey(), StorageReadBuffer(data, 1U), false);
    assertResult(StorageResult::NOT_INITIALIZED, runtime.cancel());
    assertResult(StorageResult::NOT_INITIALIZED, runtime.clearCompleted());
    runtime.begin();
    assertResult(StorageResult::INVALID_OPERATION, runtime.cancel());
    assertResult(StorageResult::SUCCESS, runtime.clearCompleted());
    runtime.submit(request);
    assertResult(StorageResult::BUSY, runtime.clearCompleted());
    backend.cancelResult = StorageBackendResult::RETRY_LATER;
    assertResult(StorageResult::BACKEND_RETRY_LATER, runtime.cancel());
    TEST_ASSERT_TRUE(runtime.isBusy()); TEST_ASSERT_EQUAL_UINT32(1U, backend.cancelCalls);
    backend.cancelResult = StorageBackendResult::SUCCESS;
    assertResult(StorageResult::CANCELLED, runtime.cancel());
    TEST_ASSERT_FALSE(runtime.isBusy()); TEST_ASSERT_EQUAL_UINT32(50U, runtime.transaction().operationId());
    assertState(StorageOperationState::CANCELLED, runtime.transaction().state());
    assertResult(StorageResult::SUCCESS, runtime.clearCompleted());
    TEST_ASSERT_FALSE(runtime.transaction().isValid());

    runtime.submit(request); backend.cancelResult = StorageBackendResult::FAILED;
    assertResult(StorageResult::BACKEND_FAILED, runtime.cancel());
    TEST_ASSERT_EQUAL_UINT32(50U, runtime.transaction().operationId());
    assertState(StorageOperationState::FAILED, runtime.transaction().state());
}

void test_completed_transaction_can_be_replaced_and_runtimes_are_independent()
{
    FakeStorageBackend firstBackend; FakeStorageBackend secondBackend;
    StorageRuntime first(firstBackend); StorageRuntime second(secondBackend);
    first.begin(); second.begin();
    uint8_t firstData[] = {1U}; uint8_t secondData[] = {2U};
    firstBackend.startResult = StorageBackendResult::SUCCESS;
    secondBackend.startResult = StorageBackendResult::SUCCESS;
    assertResult(StorageResult::SUCCESS, first.submit(StorageRequest::write(60U,
        makeKey("one/key"), StorageReadBuffer(firstData, 1U), false)));
    assertResult(StorageResult::SUCCESS, second.submit(StorageRequest::write(60U,
        makeKey("two/key"), StorageReadBuffer(secondData, 1U), false)));
    TEST_ASSERT_EQUAL_PTR(firstData, firstBackend.lastRequest.input().data());
    TEST_ASSERT_EQUAL_PTR(secondData, secondBackend.lastRequest.input().data());
    assertResult(StorageResult::SUCCESS, first.submit(StorageRequest::exists(61U, makeKey("new/key"))));
    TEST_ASSERT_EQUAL_UINT32(61U, first.transaction().operationId());
    TEST_ASSERT_EQUAL_UINT32(60U, second.transaction().operationId());
}

static_assert(std::is_base_of<StorageBackend, FakeStorageBackend>::value,
    "Fake must implement backend interface");
static_assert(std::is_trivially_copyable<StorageReadBuffer>::value,
    "Input buffer must remain a non-owning value view");
static_assert(std::is_trivially_copyable<StorageWriteBuffer>::value,
    "Output buffer must remain a non-owning value view");

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_storage_key_policy_boundaries_and_atomic_failure);
    RUN_TEST(test_buffer_views_are_bounded_non_owning_and_binary_safe);
    RUN_TEST(test_request_factories_enforce_unambiguous_operation_contracts);
    RUN_TEST(test_begin_maps_results_calls_once_and_clears_previous_state);
    RUN_TEST(test_submit_mapping_atomic_failures_and_busy_policy);
    RUN_TEST(test_update_integration_sequence_is_non_blocking_and_cumulative);
    RUN_TEST(test_start_transferred_length_contract_for_all_results_and_operations);
    RUN_TEST(test_update_maps_terminal_results_and_rejects_invalid_progress);
    RUN_TEST(test_cancel_and_clear_completed_policies_preserve_metadata);
    RUN_TEST(test_completed_transaction_can_be_replaced_and_runtimes_are_independent);
    UNITY_END();
}

void loop() {}
