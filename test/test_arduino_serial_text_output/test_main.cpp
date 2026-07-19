#include <Arduino.h>
#include <ArduinoSerialTextOutput.h>
#include <LoggerRuntime.h>
#include <SerialLogWriter.h>
#include <string.h>
#include <type_traits>
#include <unity.h>

namespace
{
constexpr size_t CAPTURE_CAPACITY = SERIAL_LOG_LINE_MAX_LENGTH;

class FakePrint final : public Print
{
public:
    size_t configuredResult = 0U;
    size_t calls = 0U;
    size_t byteCalls = 0U;
    size_t requestedLength = 0U;
    const uint8_t* receivedPointer = nullptr;
    uint8_t captured[CAPTURE_CAPACITY] = {};

    size_t write(uint8_t value) override
    {
        ++byteCalls;
        captured[0] = value;
        return configuredResult;
    }

    size_t write(const uint8_t* data, size_t length) override
    {
        ++calls;
        requestedLength = length;
        receivedPointer = data;
        memset(captured, 0, sizeof(captured));
        const size_t copyLength = length < sizeof(captured) ? length : sizeof(captured);
        if (data != nullptr) memcpy(captured, data, copyLength);
        return configuredResult;
    }
};

LogRecord makeRecord(LogId id = 42U)
{
    LogRecord record;
    record.id = id;
    record.timestampMs = 12500U;
    record.level = LogLevel::WARNING;
    record.category = LogCategory::SECURITY;
    record.sourceType = LogSourceType::COMMAND;
    record.sourceId = 7U;
    record.code = 202U;
    record.payload.value1 = 10U;
    record.payload.value2 = 20U;
    record.payload.signedValue = -3;
    record.payload.flag = true;
    record.setMessage("command rejected");
    return record;
}

void assertTextResult(TextOutputResult expected, TextOutputResult actual)
{ TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected), static_cast<uint8_t>(actual)); }
void assertLogResult(LogWriteResult expected, LogWriteResult actual)
{ TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected), static_cast<uint8_t>(actual)); }
void assertUpdateResult(LoggerUpdateResult expected, LoggerUpdateResult actual)
{ TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected), static_cast<uint8_t>(actual)); }
}

void test_valid_write_is_exact_and_uses_buffer_override_once()
{
    FakePrint output;
    ArduinoSerialTextOutput adapter(output);
    TextOutput& interfaceReference = adapter;
    char data[] = {'A', 'B', 'C', '\0', 'X'};
    const char original[] = {'A', 'B', 'C', '\0', 'X'};
    output.configuredResult = sizeof(data);

    assertTextResult(TextOutputResult::SUCCESS,
        interfaceReference.write(data, sizeof(data)));

    TEST_ASSERT_EQUAL_UINT32(1U, output.calls);
    TEST_ASSERT_EQUAL_UINT32(0U, output.byteCalls);
    TEST_ASSERT_EQUAL_UINT32(sizeof(data), output.requestedLength);
    TEST_ASSERT_EQUAL_PTR(reinterpret_cast<const uint8_t*>(data), output.receivedPointer);
    TEST_ASSERT_EQUAL_MEMORY(original, output.captured, sizeof(data));
    TEST_ASSERT_EQUAL_MEMORY(original, data, sizeof(data));
}

void test_full_zero_partial_and_invalid_print_results_are_fail_closed()
{
    const char data[] = "line\n";
    const size_t length = sizeof(data) - 1U;
    FakePrint output;
    ArduinoSerialTextOutput adapter(output);

    output.configuredResult = length;
    assertTextResult(TextOutputResult::SUCCESS, adapter.write(data, length));
    output.configuredResult = 0U;
    assertTextResult(TextOutputResult::RETRY_LATER, adapter.write(data, length));
    output.configuredResult = length - 1U;
    assertTextResult(TextOutputResult::FAILED, adapter.write(data, length));
    output.configuredResult = length + 1U;
    assertTextResult(TextOutputResult::FAILED, adapter.write(data, length));

    TEST_ASSERT_EQUAL_UINT32(4U, output.calls);
    TEST_ASSERT_EQUAL_UINT32(0U, output.byteCalls);
    TEST_ASSERT_EQUAL_UINT32(length, output.requestedLength);
    TEST_ASSERT_EQUAL_MEMORY(data, output.captured, length);
    TEST_ASSERT_EQUAL_UINT8(0U, output.captured[length]);
}

void test_invalid_arguments_do_not_call_print()
{
    FakePrint output;
    ArduinoSerialTextOutput adapter(output);
    const char data[] = "valid";

    assertTextResult(TextOutputResult::FAILED, adapter.write(nullptr, 1U));
    assertTextResult(TextOutputResult::FAILED, adapter.write(data, 0U));
    assertTextResult(TextOutputResult::FAILED, adapter.write(nullptr, 0U));
    TEST_ASSERT_EQUAL_UINT32(0U, output.calls);
    TEST_ASSERT_EQUAL_UINT32(0U, output.byteCalls);
}

void test_binary_utf8_and_delimiters_are_transferred_byte_for_byte()
{
    const char data[] = {'A', '\0', '|', '\n', '\r',
        static_cast<char>(0xD8), static_cast<char>(0xAF), 'Z'};
    FakePrint output;
    output.configuredResult = sizeof(data);
    ArduinoSerialTextOutput adapter(output);

    assertTextResult(TextOutputResult::SUCCESS, adapter.write(data, sizeof(data)));
    TEST_ASSERT_EQUAL_UINT32(sizeof(data), output.requestedLength);
    TEST_ASSERT_EQUAL_MEMORY(data, output.captured, sizeof(data));
}

void test_multiple_adapters_keep_outputs_independent()
{
    FakePrint first;
    FakePrint second;
    first.configuredResult = 3U;
    second.configuredResult = 4U;
    ArduinoSerialTextOutput firstAdapter(first);
    ArduinoSerialTextOutput secondAdapter(second);

    assertTextResult(TextOutputResult::SUCCESS, firstAdapter.write("one", 3U));
    assertTextResult(TextOutputResult::SUCCESS, secondAdapter.write("two!", 4U));
    TEST_ASSERT_EQUAL_MEMORY("one", first.captured, 3U);
    TEST_ASSERT_EQUAL_MEMORY("two!", second.captured, 4U);
    TEST_ASSERT_EQUAL_UINT32(1U, first.calls);
    TEST_ASSERT_EQUAL_UINT32(1U, second.calls);
}

void test_serial_log_writer_pipeline_maps_all_write_outcomes()
{
    const char expected[] = "LOG|id=42|ts=12500|level=WARNING|cat=SECURITY|src=COMMAND|sid=7|code=202|v1=10|v2=20|sv=-3|flag=1|msg=command rejected\n";
    FakePrint output;
    ArduinoSerialTextOutput adapter(output);
    SerialLogWriter writer(adapter);
    const LogRecord record = makeRecord();
    const size_t length = sizeof(expected) - 1U;

    output.configuredResult = length;
    assertLogResult(LogWriteResult::WRITTEN, writer.write(record));
    TEST_ASSERT_EQUAL_UINT32(1U, output.calls);
    TEST_ASSERT_EQUAL_UINT32(length, output.requestedLength);
    TEST_ASSERT_EQUAL_MEMORY(expected, output.captured, length);
    output.configuredResult = 0U;
    assertLogResult(LogWriteResult::RETRY_LATER, writer.write(record));
    output.configuredResult = length - 1U;
    assertLogResult(LogWriteResult::FAILED, writer.write(record));
    TEST_ASSERT_EQUAL_UINT32(3U, output.calls);
}

void test_logger_runtime_retries_only_zero_write_and_consumes_partial_failure()
{
    const size_t formattedLength = sizeof("LOG|id=42|ts=12500|level=WARNING|cat=SECURITY|src=COMMAND|sid=7|code=202|v1=10|v2=20|sv=-3|flag=1|msg=command rejected\n") - 1U;
    LogQueue queue;
    LoggerRuntime runtime(queue);
    FakePrint output;
    ArduinoSerialTextOutput adapter(output);
    SerialLogWriter writer(adapter);
    runtime.addWriter(writer);

    output.configuredResult = formattedLength;
    queue.publish(makeRecord(42U));
    assertUpdateResult(LoggerUpdateResult::SUCCESS, runtime.update());
    TEST_ASSERT_TRUE(queue.empty());

    output.configuredResult = 0U;
    queue.publish(makeRecord(43U));
    assertUpdateResult(LoggerUpdateResult::RETRY_LATER, runtime.update());
    TEST_ASSERT_EQUAL_UINT32(1U, queue.size());
    TEST_ASSERT_EQUAL_UINT32(2U, output.calls);
    output.configuredResult = formattedLength;
    assertUpdateResult(LoggerUpdateResult::SUCCESS, runtime.update());
    TEST_ASSERT_TRUE(queue.empty());
    TEST_ASSERT_EQUAL_UINT32(3U, output.calls);

    output.configuredResult = formattedLength - 1U;
    queue.publish(makeRecord(44U));
    assertUpdateResult(LoggerUpdateResult::WRITER_FAILED, runtime.update());
    TEST_ASSERT_TRUE(queue.empty());
    TEST_ASSERT_EQUAL_UINT32(4U, output.calls);
    assertUpdateResult(LoggerUpdateResult::QUEUE_EMPTY, runtime.update());
    TEST_ASSERT_EQUAL_UINT32(4U, output.calls);
}

static_assert(std::is_base_of<TextOutput, ArduinoSerialTextOutput>::value,
    "Adapter must implement TextOutput");
static_assert(sizeof(TextOutput*) == 4U, "Unexpected ESP32 pointer size");
static_assert(sizeof(Print*) == 4U, "Unexpected ESP32 Print pointer size");
static_assert(sizeof(ArduinoSerialTextOutput) == 8U,
    "Unexpected adapter ABI size");

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_valid_write_is_exact_and_uses_buffer_override_once);
    RUN_TEST(test_full_zero_partial_and_invalid_print_results_are_fail_closed);
    RUN_TEST(test_invalid_arguments_do_not_call_print);
    RUN_TEST(test_binary_utf8_and_delimiters_are_transferred_byte_for_byte);
    RUN_TEST(test_multiple_adapters_keep_outputs_independent);
    RUN_TEST(test_serial_log_writer_pipeline_maps_all_write_outcomes);
    RUN_TEST(test_logger_runtime_retries_only_zero_write_and_consumes_partial_failure);
    UNITY_END();
}

void loop() {}
