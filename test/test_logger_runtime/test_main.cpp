#include <Arduino.h>
#include <LoggerRuntime.h>
#include <type_traits>
#include <unity.h>

class LogQueueTestAccess
{
public:
    static void invalidateHead(LogQueue& queue) { queue.records_[queue.head_].id = INVALID_LOG_ID; }
    static void makeConsumeFail(LogQueue& queue)
    { queue.count_ = 0U; queue.head_ = queue.tail_; }
};

namespace
{
LogRecord makeRecord(LogId id = 1U)
{
    LogRecord record;
    record.id = id;
    record.timestampMs = 100U + id;
    record.level = LogLevel::INFO;
    record.category = LogCategory::DIAGNOSTIC;
    record.sourceType = LogSourceType::DEVICE;
    record.sourceId = 7U;
    record.code = 12U;
    record.payload.value1 = id * 10U;
    return record;
}

class FakeLogWriter : public LogWriter
{
public:
    LogWriteResult result = LogWriteResult::WRITTEN;
    size_t calls = 0U;
    LogRecord lastRecord{};
    size_t* sequence = nullptr;
    size_t calledAt = 0U;
    LogWriteResult write(const LogRecord& record) override
    {
        ++calls; lastRecord = record;
        if (sequence != nullptr) calledAt = ++(*sequence);
        return result;
    }
};

class RecursiveLogWriter final : public FakeLogWriter
{
public:
    explicit RecursiveLogWriter(LoggerRuntime& runtime) : runtime_(runtime) {}
    LoggerUpdateResult nestedResult = LoggerUpdateResult::SUCCESS;
    LogWriteResult write(const LogRecord& record) override
    {
        const LogWriteResult configured = FakeLogWriter::write(record);
        nestedResult = runtime_.update();
        return configured;
    }
private:
    LoggerRuntime& runtime_;
};

class PublishingLogWriter final : public FakeLogWriter
{
public:
    explicit PublishingLogWriter(LogSink& sink) : sink_(sink) {}
    LogRecord recordToPublish = makeRecord(99U);
    LogPublishResult publishResult = LogPublishResult::INVALID_RECORD;
    LogWriteResult write(const LogRecord& record) override
    {
        const LogWriteResult configured = FakeLogWriter::write(record);
        publishResult = sink_.publish(recordToPublish);
        return configured;
    }
private:
    LogSink& sink_;
};

class ConsumeFailingLogWriter final : public LogWriter
{
public:
    explicit ConsumeFailingLogWriter(LogQueue& queue) : queue_(queue) {}
    LogWriteResult write(const LogRecord&) override
    { LogQueueTestAccess::makeConsumeFail(queue_); return LogWriteResult::WRITTEN; }
private:
    LogQueue& queue_;
};

void assertPublish(LogPublishResult expected, LogPublishResult actual)
{ TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected), static_cast<uint8_t>(actual)); }
void assertUpdate(LoggerUpdateResult expected, LoggerUpdateResult actual)
{ TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected), static_cast<uint8_t>(actual)); }
}

void test_log_model_validity_payload_message_and_copy()
{
    LogRecord record; TEST_ASSERT_FALSE(record.isValid()); TEST_ASSERT_EQUAL_STRING("", record.message);
    TEST_ASSERT_EQUAL_UINT32(0U, record.payload.value1); TEST_ASSERT_EQUAL_UINT32(0U, record.payload.value2);
    TEST_ASSERT_EQUAL_INT32(0, record.payload.signedValue); TEST_ASSERT_FALSE(record.payload.flag);
    record = makeRecord(); TEST_ASSERT_TRUE(record.isValid()); TEST_ASSERT_TRUE(record.setMessage("ready"));
    TEST_ASSERT_EQUAL_STRING("ready", record.message); char previous[LOG_MESSAGE_MAX_LENGTH]; memcpy(previous, record.message, sizeof(previous));
    TEST_ASSERT_FALSE(record.setMessage(nullptr)); TEST_ASSERT_EQUAL_MEMORY(previous, record.message, sizeof(previous));
    char tooLong[LOG_MESSAGE_MAX_LENGTH]; memset(tooLong, 'x', sizeof(tooLong));
    TEST_ASSERT_FALSE(record.setMessage(tooLong)); TEST_ASSERT_EQUAL_MEMORY(previous, record.message, sizeof(previous));
    LogRecord copy = record; record.message[0] = 'R'; record.payload.value1 = 500U;
    TEST_ASSERT_EQUAL_STRING("ready", copy.message); TEST_ASSERT_EQUAL_UINT32(10U, copy.payload.value1);
    copy.id = INVALID_LOG_ID; TEST_ASSERT_FALSE(copy.isValid());
    copy = makeRecord(); copy.level = LogLevel::COUNT; TEST_ASSERT_FALSE(copy.isValid());
    copy = makeRecord(); copy.level = static_cast<LogLevel>(255U); TEST_ASSERT_FALSE(copy.isValid());
    copy = makeRecord(); copy.category = static_cast<LogCategory>(255U); TEST_ASSERT_FALSE(copy.isValid());
    copy = makeRecord(); copy.sourceType = static_cast<LogSourceType>(255U); TEST_ASSERT_FALSE(copy.isValid());
    copy = makeRecord(); copy.sourceId = 0U; TEST_ASSERT_FALSE(copy.isValid());
    copy = makeRecord(); copy.sourceType = LogSourceType::SYSTEM; copy.sourceId = 0U; TEST_ASSERT_TRUE(copy.isValid());
}

void test_queue_publish_validation_duplicate_full_and_copy()
{
    LogQueue queue; TEST_ASSERT_TRUE(queue.empty()); TEST_ASSERT_FALSE(queue.full());
    TEST_ASSERT_EQUAL_UINT32(LOG_QUEUE_CAPACITY, queue.capacity()); TEST_ASSERT_NULL(queue.peek());
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(LogQueueResult::QUEUE_EMPTY), static_cast<uint8_t>(queue.consume()));
    LogRecord record = makeRecord(); assertPublish(LogPublishResult::SUCCESS, queue.publish(record));
    assertPublish(LogPublishResult::DUPLICATE_LOG_ID, queue.publish(record));
    LogRecord invalid; assertPublish(LogPublishResult::INVALID_RECORD, queue.publish(invalid)); TEST_ASSERT_EQUAL_UINT32(1U, queue.size());
    record.payload.value1 = 900U; TEST_ASSERT_EQUAL_UINT32(10U, queue.peek()->payload.value1);
    for (LogId id = 2U; id <= LOG_QUEUE_CAPACITY; ++id) assertPublish(LogPublishResult::SUCCESS, queue.publish(makeRecord(id)));
    const size_t before = queue.size(); assertPublish(LogPublishResult::QUEUE_FULL, queue.publish(makeRecord(100U)));
    TEST_ASSERT_EQUAL_UINT32(before, queue.size()); TEST_ASSERT_TRUE(queue.full());
}

void test_queue_fifo_wraparound_clear_and_reuse_id()
{
    LogQueue queue;
    for (LogId id = 1U; id <= LOG_QUEUE_CAPACITY; ++id) queue.publish(makeRecord(id));
    for (LogId id = 1U; id <= 8U; ++id) { TEST_ASSERT_EQUAL_UINT32(id, queue.peek()->id); queue.consume(); }
    for (LogId id = LOG_QUEUE_CAPACITY + 1U; id <= LOG_QUEUE_CAPACITY + 8U; ++id) queue.publish(makeRecord(id));
    for (LogId id = 9U; id <= LOG_QUEUE_CAPACITY + 8U; ++id) { TEST_ASSERT_EQUAL_UINT32(id, queue.peek()->id); queue.consume(); }
    assertPublish(LogPublishResult::SUCCESS, queue.publish(makeRecord(1U))); queue.consume();
    assertPublish(LogPublishResult::SUCCESS, queue.publish(makeRecord(1U))); queue.clear();
    TEST_ASSERT_TRUE(queue.empty()); TEST_ASSERT_NULL(queue.peek());
}

void test_writer_registration_removal_and_order()
{
    LogQueue queue; LoggerRuntime runtime(queue); FakeLogWriter writers[LOG_WRITER_CAPACITY + 1U];
    TEST_ASSERT_EQUAL_UINT32(LOG_WRITER_CAPACITY, runtime.writerCapacity());
    for (size_t index = 0U; index < LOG_WRITER_CAPACITY; ++index)
        TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(runtime.addWriter(writers[index])));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(LogWriterRegistrationResult::DUPLICATE_WRITER), static_cast<uint8_t>(runtime.addWriter(writers[0])));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(LogWriterRegistrationResult::REGISTRY_FULL), static_cast<uint8_t>(runtime.addWriter(writers[LOG_WRITER_CAPACITY])));
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(runtime.removeWriter(writers[1])));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(LogWriterRegistrationResult::WRITER_NOT_FOUND), static_cast<uint8_t>(runtime.removeWriter(writers[1])));
    size_t sequence = 0U; for (size_t index = 0U; index < LOG_WRITER_CAPACITY; ++index) writers[index].sequence = &sequence;
    queue.publish(makeRecord()); assertUpdate(LoggerUpdateResult::SUCCESS, runtime.update());
    TEST_ASSERT_EQUAL_UINT32(0U, writers[1].calls); TEST_ASSERT_EQUAL_UINT32(2U, writers[2].calledAt);
}

void test_empty_no_writers_success_ignored_and_next_record()
{
    LogQueue queue; LoggerRuntime runtime(queue); FakeLogWriter written; FakeLogWriter ignored;
    assertUpdate(LoggerUpdateResult::QUEUE_EMPTY, runtime.update());
    queue.publish(makeRecord()); assertUpdate(LoggerUpdateResult::NO_WRITERS, runtime.update()); TEST_ASSERT_TRUE(queue.empty());
    runtime.addWriter(written); runtime.addWriter(ignored); ignored.result = LogWriteResult::IGNORED;
    queue.publish(makeRecord(2U)); queue.publish(makeRecord(3U));
    assertUpdate(LoggerUpdateResult::SUCCESS, runtime.update()); TEST_ASSERT_EQUAL_UINT32(2U, written.lastRecord.id);
    TEST_ASSERT_EQUAL_UINT32(3U, queue.peek()->id); written.result = LogWriteResult::IGNORED;
    assertUpdate(LoggerUpdateResult::RECORD_IGNORED, runtime.update()); TEST_ASSERT_TRUE(queue.empty());
}

void test_failure_invalid_result_and_other_writers_continue()
{
    LogQueue queue; LoggerRuntime runtime(queue); FakeLogWriter failed; FakeLogWriter later;
    failed.result = LogWriteResult::FAILED; runtime.addWriter(failed); runtime.addWriter(later);
    queue.publish(makeRecord()); assertUpdate(LoggerUpdateResult::WRITER_FAILED, runtime.update());
    TEST_ASSERT_EQUAL_UINT32(1U, failed.calls); TEST_ASSERT_EQUAL_UINT32(1U, later.calls); TEST_ASSERT_TRUE(queue.empty());
    failed.result = static_cast<LogWriteResult>(255U); queue.publish(makeRecord(2U));
    assertUpdate(LoggerUpdateResult::WRITER_FAILED, runtime.update()); TEST_ASSERT_EQUAL_UINT32(2U, later.calls);
    failed.result = LogWriteResult::WRITTEN; queue.publish(makeRecord(3U)); assertUpdate(LoggerUpdateResult::SUCCESS, runtime.update());
}

void test_retry_later_retains_head_retries_all_and_preserves_fifo()
{
    LogQueue queue; LoggerRuntime runtime(queue); FakeLogWriter written; FakeLogWriter retry;
    retry.result = LogWriteResult::RETRY_LATER; runtime.addWriter(written); runtime.addWriter(retry);
    queue.publish(makeRecord()); queue.publish(makeRecord(2U)); const size_t before = queue.size();
    assertUpdate(LoggerUpdateResult::RETRY_LATER, runtime.update()); TEST_ASSERT_EQUAL_UINT32(before, queue.size());
    TEST_ASSERT_EQUAL_UINT32(1U, queue.peek()->id); TEST_ASSERT_EQUAL_UINT32(1U, written.calls);
    assertUpdate(LoggerUpdateResult::RETRY_LATER, runtime.update()); TEST_ASSERT_EQUAL_UINT32(2U, written.calls);
    retry.result = LogWriteResult::IGNORED; assertUpdate(LoggerUpdateResult::SUCCESS, runtime.update());
    TEST_ASSERT_EQUAL_UINT32(2U, queue.peek()->id);
}

void test_retry_overrides_failure_and_invalid_result()
{
    LogQueue queue; LoggerRuntime runtime(queue); FakeLogWriter first; FakeLogWriter retry;
    first.result = LogWriteResult::FAILED; retry.result = LogWriteResult::RETRY_LATER;
    runtime.addWriter(first); runtime.addWriter(retry); queue.publish(makeRecord());
    assertUpdate(LoggerUpdateResult::RETRY_LATER, runtime.update()); TEST_ASSERT_EQUAL_UINT32(1U, queue.size());
    first.result = static_cast<LogWriteResult>(255U); assertUpdate(LoggerUpdateResult::RETRY_LATER, runtime.update());
    TEST_ASSERT_EQUAL_UINT32(2U, first.calls); TEST_ASSERT_EQUAL_UINT32(2U, retry.calls);
}

void test_recursive_update_rejected_original_continues_and_guard_resets()
{
    LogQueue queue; LoggerRuntime runtime(queue); RecursiveLogWriter recursive(runtime); FakeLogWriter later;
    size_t sequence = 0U; recursive.sequence = &sequence; later.sequence = &sequence;
    runtime.addWriter(recursive); runtime.addWriter(later); queue.publish(makeRecord());
    assertUpdate(LoggerUpdateResult::SUCCESS, runtime.update());
    assertUpdate(LoggerUpdateResult::REENTRANT_CALL, recursive.nestedResult);
    TEST_ASSERT_EQUAL_UINT32(1U, recursive.calls); TEST_ASSERT_EQUAL_UINT32(1U, recursive.calledAt);
    TEST_ASSERT_EQUAL_UINT32(1U, later.calls); TEST_ASSERT_EQUAL_UINT32(2U, later.calledAt); TEST_ASSERT_TRUE(queue.empty());
    recursive.result = LogWriteResult::FAILED; queue.publish(makeRecord(2U));
    assertUpdate(LoggerUpdateResult::WRITER_FAILED, runtime.update());
    recursive.result = LogWriteResult::RETRY_LATER; queue.publish(makeRecord(3U));
    assertUpdate(LoggerUpdateResult::RETRY_LATER, runtime.update());
    recursive.result = LogWriteResult::WRITTEN; assertUpdate(LoggerUpdateResult::SUCCESS, runtime.update());
}

void test_reentrant_publish_deferred_and_full_policy()
{
    LogQueue queue; LoggerRuntime runtime(queue); PublishingLogWriter publisher(queue); runtime.addWriter(publisher);
    queue.publish(makeRecord()); assertUpdate(LoggerUpdateResult::SUCCESS, runtime.update());
    assertPublish(LogPublishResult::SUCCESS, publisher.publishResult); TEST_ASSERT_EQUAL_UINT32(99U, queue.peek()->id);
    assertUpdate(LoggerUpdateResult::SUCCESS, runtime.update()); TEST_ASSERT_TRUE(queue.empty());
    queue.clear(); for (LogId id = 1U; id <= LOG_QUEUE_CAPACITY; ++id) queue.publish(makeRecord(id));
    publisher.recordToPublish = makeRecord(500U); runtime.update();
    assertPublish(LogPublishResult::QUEUE_FULL, publisher.publishResult); TEST_ASSERT_EQUAL_UINT32(LOG_QUEUE_CAPACITY - 1U, queue.size());
}

void test_corrupt_head_and_internal_error_recovery()
{
    LogQueue queue; LoggerRuntime runtime(queue); FakeLogWriter writer; runtime.addWriter(writer);
    queue.publish(makeRecord()); queue.publish(makeRecord(2U)); LogQueueTestAccess::invalidateHead(queue);
    assertUpdate(LoggerUpdateResult::INVALID_RECORD, runtime.update()); TEST_ASSERT_EQUAL_UINT32(0U, writer.calls);
    TEST_ASSERT_EQUAL_UINT32(2U, queue.peek()->id); assertUpdate(LoggerUpdateResult::SUCCESS, runtime.update());

    LogQueue internalQueue; LoggerRuntime internalRuntime(internalQueue); ConsumeFailingLogWriter corruptor(internalQueue);
    internalRuntime.addWriter(corruptor); internalQueue.publish(makeRecord());
    assertUpdate(LoggerUpdateResult::INTERNAL_ERROR, internalRuntime.update());
    internalRuntime.removeWriter(corruptor); FakeLogWriter recovered; internalRuntime.addWriter(recovered);
    internalQueue.publish(makeRecord(2U)); assertUpdate(LoggerUpdateResult::SUCCESS, internalRuntime.update());
}

static_assert(std::is_base_of<LogSink, LogQueue>::value, "Producer باید فقط LogSink را ببیند");
static_assert(std::is_same<decltype(static_cast<const LogQueue&>(*static_cast<LogQueue*>(nullptr)).peek()), const LogRecord*>::value, "peek باید فقط const Record بدهد");
static_assert(std::is_trivially_copyable<LogPayload>::value, "Payload باید Copy ساده داشته باشد");

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_log_model_validity_payload_message_and_copy);
    RUN_TEST(test_queue_publish_validation_duplicate_full_and_copy);
    RUN_TEST(test_queue_fifo_wraparound_clear_and_reuse_id);
    RUN_TEST(test_writer_registration_removal_and_order);
    RUN_TEST(test_empty_no_writers_success_ignored_and_next_record);
    RUN_TEST(test_failure_invalid_result_and_other_writers_continue);
    RUN_TEST(test_retry_later_retains_head_retries_all_and_preserves_fifo);
    RUN_TEST(test_retry_overrides_failure_and_invalid_result);
    RUN_TEST(test_recursive_update_rejected_original_continues_and_guard_resets);
    RUN_TEST(test_reentrant_publish_deferred_and_full_policy);
    RUN_TEST(test_corrupt_head_and_internal_error_recovery);
    UNITY_END();
}
void loop() {}
