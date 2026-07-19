#include <Arduino.h>
#include <EventDispatcher.h>
#include <EventToLoggerAdapter.h>
#include <LoggerRuntime.h>
#include <SequentialLogIdProvider.h>
#include <SerialLogWriter.h>
#include <string.h>
#include <type_traits>
#include <unity.h>

namespace
{
LogRecord makeRecord()
{
    LogRecord record;
    record.id = 42U;
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

Event makeEvent(EventId id = 1U)
{
    Event event;
    event.id = id;
    event.type = EventType::COMMAND_REJECTED;
    event.severity = EventSeverity::WARNING;
    event.sourceType = EventSourceType::COMMAND;
    event.sourceId = 7U;
    event.timestampMs = 12500U;
    event.payload.value1 = 10U;
    event.payload.value2 = 20U;
    event.payload.signedValue = -3;
    event.payload.flag = true;
    event.setMessage("command rejected");
    return event;
}

class FakeTextOutput final : public TextOutput
{
public:
    TextOutputResult result = TextOutputResult::SUCCESS;
    size_t calls = 0U;
    size_t lastLength = 0U;
    char lastData[SERIAL_LOG_LINE_MAX_LENGTH] = {};
    TextOutputResult write(const char* data, size_t length) override
    {
        ++calls; lastLength = length;
        memset(lastData, 0, sizeof(lastData));
        if (data != nullptr && length < sizeof(lastData)) memcpy(lastData, data, length);
        return result;
    }
};

class FakeLogWriter final : public LogWriter
{
public:
    size_t calls = 0U;
    LogWriteResult result = LogWriteResult::WRITTEN;
    LogWriteResult write(const LogRecord&) override { ++calls; return result; }
};

void assertFormat(LogFormatResult expected, LogFormatResult actual)
{ TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected), static_cast<uint8_t>(actual)); }
void assertWrite(LogWriteResult expected, LogWriteResult actual)
{ TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected), static_cast<uint8_t>(actual)); }
void assertUpdate(LoggerUpdateResult expected, LoggerUpdateResult actual)
{ TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected), static_cast<uint8_t>(actual)); }
}

void test_formatter_exact_output_and_reuse()
{
    const char expected[] = "LOG|id=42|ts=12500|level=WARNING|cat=SECURITY|src=COMMAND|sid=7|code=202|v1=10|v2=20|sv=-3|flag=1|msg=command rejected\n";
    LogRecordFormatter formatter; LogRecord record = makeRecord(); const LogRecord original = record;
    char output[SERIAL_LOG_LINE_MAX_LENGTH]; size_t length = 99U;
    assertFormat(LogFormatResult::SUCCESS, formatter.format(record, output, sizeof(output), length));
    TEST_ASSERT_EQUAL_STRING(expected, output); TEST_ASSERT_EQUAL_UINT32(strlen(expected), length);
    TEST_ASSERT_EQUAL_INT8('\n', output[length - 1U]); TEST_ASSERT_EQUAL_INT8('\0', output[length]);
    TEST_ASSERT_EQUAL_UINT32(original.id, record.id); TEST_ASSERT_EQUAL_STRING(original.message, record.message);
    record.id = 43U; record.message[0] = '\0';
    assertFormat(LogFormatResult::SUCCESS, formatter.format(record, output, sizeof(output), length));
    TEST_ASSERT_NOT_NULL(strstr(output, "LOG|id=43|")); TEST_ASSERT_NOT_NULL(strstr(output, "|msg=\n"));
}

void test_unsigned_and_signed_extremes()
{
    LogRecordFormatter formatter; LogRecord record = makeRecord(); char output[SERIAL_LOG_LINE_MAX_LENGTH]; size_t length;
    record.id = UINT32_MAX; record.timestampMs = UINT32_MAX; record.sourceId = UINT32_MAX;
    record.code = UINT16_MAX; record.payload.value1 = UINT32_MAX; record.payload.value2 = 0U;
    record.payload.signedValue = INT32_MIN;
    assertFormat(LogFormatResult::SUCCESS, formatter.format(record, output, sizeof(output), length));
    TEST_ASSERT_NOT_NULL(strstr(output, "id=4294967295")); TEST_ASSERT_NOT_NULL(strstr(output, "ts=4294967295"));
    TEST_ASSERT_NOT_NULL(strstr(output, "sid=4294967295")); TEST_ASSERT_NOT_NULL(strstr(output, "code=65535"));
    TEST_ASSERT_NOT_NULL(strstr(output, "v1=4294967295|v2=0|sv=-2147483648"));
    record.payload.signedValue = INT32_MAX; formatter.format(record, output, sizeof(output), length);
    TEST_ASSERT_NOT_NULL(strstr(output, "|sv=2147483647|"));
    record.payload.signedValue = 0; formatter.format(record, output, sizeof(output), length);
    TEST_ASSERT_NOT_NULL(strstr(output, "|sv=0|"));
}

void test_all_enum_names_and_invalid_enums()
{
    const LogLevel levels[] = { LogLevel::TRACE, LogLevel::DEBUG, LogLevel::INFO,
        LogLevel::NOTICE, LogLevel::WARNING, LogLevel::ERROR, LogLevel::CRITICAL };
    const char* levelNames[] = { "TRACE", "DEBUG", "INFO", "NOTICE", "WARNING", "ERROR", "CRITICAL" };
    const LogCategory categories[] = { LogCategory::SYSTEM, LogCategory::COMMAND, LogCategory::SECURITY,
        LogCategory::DEVICE, LogCategory::NODE, LogCategory::STORAGE, LogCategory::TRANSPORT,
        LogCategory::AUTOMATION, LogCategory::CONFIGURATION, LogCategory::DIAGNOSTIC };
    const char* categoryNames[] = { "SYSTEM", "COMMAND", "SECURITY", "DEVICE", "NODE", "STORAGE",
        "TRANSPORT", "AUTOMATION", "CONFIGURATION", "DIAGNOSTIC" };
    const LogSourceType sources[] = { LogSourceType::SYSTEM, LogSourceType::COMMAND, LogSourceType::DEVICE,
        LogSourceType::ENDPOINT, LogSourceType::NODE, LogSourceType::TRANSPORT, LogSourceType::STORAGE,
        LogSourceType::RULE, LogSourceType::SCENE, LogSourceType::SCHEDULER, LogSourceType::SECURITY, LogSourceType::USER };
    const char* sourceNames[] = { "SYSTEM", "COMMAND", "DEVICE", "ENDPOINT", "NODE", "TRANSPORT",
        "STORAGE", "RULE", "SCENE", "SCHEDULER", "SECURITY", "USER" };
    LogRecordFormatter formatter; char output[SERIAL_LOG_LINE_MAX_LENGTH]; size_t length; LogRecord record = makeRecord();
    for (size_t index = 0U; index < 7U; ++index)
    {
        record.level = levels[index]; assertFormat(LogFormatResult::SUCCESS, formatter.format(record, output, sizeof(output), length));
        char expected[24] = "|level="; strcat(expected, levelNames[index]); strcat(expected, "|"); TEST_ASSERT_NOT_NULL(strstr(output, expected));
    }
    for (size_t index = 0U; index < 10U; ++index)
    {
        record.category = categories[index]; assertFormat(LogFormatResult::SUCCESS, formatter.format(record, output, sizeof(output), length));
        char expected[32] = "|cat="; strcat(expected, categoryNames[index]); strcat(expected, "|"); TEST_ASSERT_NOT_NULL(strstr(output, expected));
    }
    record.category = LogCategory::SECURITY;
    for (size_t index = 0U; index < 12U; ++index)
    {
        record.sourceType = sources[index]; record.sourceId = sources[index] == LogSourceType::SYSTEM ? 0U : 1U;
        assertFormat(LogFormatResult::SUCCESS, formatter.format(record, output, sizeof(output), length));
        char expected[28] = "|src="; strcat(expected, sourceNames[index]); strcat(expected, "|"); TEST_ASSERT_NOT_NULL(strstr(output, expected));
    }
    record = makeRecord(); record.level = LogLevel::COUNT; assertFormat(LogFormatResult::INVALID_RECORD, formatter.format(record, output, sizeof(output), length));
    record = makeRecord(); record.category = static_cast<LogCategory>(255U); assertFormat(LogFormatResult::INVALID_RECORD, formatter.format(record, output, sizeof(output), length));
    record = makeRecord(); record.sourceType = LogSourceType::COUNT; assertFormat(LogFormatResult::INVALID_RECORD, formatter.format(record, output, sizeof(output), length));
}

void test_message_validation_boundary_and_utf8()
{
    LogRecordFormatter formatter; char output[SERIAL_LOG_LINE_MAX_LENGTH]; size_t length; LogRecord record = makeRecord();
    memset(record.message, 'a', sizeof(record.message)); record.message[sizeof(record.message) - 1U] = '\0';
    assertFormat(LogFormatResult::SUCCESS, formatter.format(record, output, sizeof(output), length));
    TEST_ASSERT_EQUAL_INT8('\n', output[length - 1U]);
    record.setMessage("bad|message"); assertFormat(LogFormatResult::UNSUPPORTED_MESSAGE, formatter.format(record, output, sizeof(output), length));
    TEST_ASSERT_EQUAL_UINT32(0U, length); TEST_ASSERT_EQUAL_STRING("", output);
    record.setMessage("bad\nmessage"); assertFormat(LogFormatResult::UNSUPPORTED_MESSAGE, formatter.format(record, output, sizeof(output), length));
    record.setMessage("bad\rmessage"); assertFormat(LogFormatResult::UNSUPPORTED_MESSAGE, formatter.format(record, output, sizeof(output), length));
    const char utf8[] = "\xD8\xAF\xD9\x84\xD8\xB3\xD8\xA7\xD9\x85";
    record.setMessage(utf8); assertFormat(LogFormatResult::SUCCESS, formatter.format(record, output, sizeof(output), length));
    TEST_ASSERT_NOT_NULL(strstr(output, utf8));
}

void test_invalid_arguments_records_and_failure_cleanup()
{
    LogRecordFormatter formatter; char output[32]; memset(output, 'x', sizeof(output)); size_t length = 88U;
    LogRecord record = makeRecord();
    assertFormat(LogFormatResult::INVALID_ARGUMENT, formatter.format(record, nullptr, sizeof(output), length)); TEST_ASSERT_EQUAL_UINT32(0U, length);
    assertFormat(LogFormatResult::INVALID_ARGUMENT, formatter.format(record, output, 0U, length));
    LogRecord invalid; assertFormat(LogFormatResult::INVALID_RECORD, formatter.format(invalid, output, sizeof(output), length));
    TEST_ASSERT_EQUAL_STRING("", output); TEST_ASSERT_EQUAL_UINT32(0U, length);
    invalid = makeRecord(); invalid.id = INVALID_LOG_ID; assertFormat(LogFormatResult::INVALID_RECORD, formatter.format(invalid, output, sizeof(output), length));
    invalid = makeRecord(); invalid.sourceId = 0U; assertFormat(LogFormatResult::INVALID_RECORD, formatter.format(invalid, output, sizeof(output), length));
    invalid = makeRecord(); memset(invalid.message, 'x', sizeof(invalid.message));
    assertFormat(LogFormatResult::INVALID_RECORD, formatter.format(invalid, output, sizeof(output), length));
}

void test_buffer_boundaries_do_not_overwrite_guard()
{
    LogRecordFormatter formatter; LogRecord record = makeRecord(); char large[SERIAL_LOG_LINE_MAX_LENGTH]; size_t needed;
    assertFormat(LogFormatResult::SUCCESS, formatter.format(record, large, sizeof(large), needed));
    char exact[SERIAL_LOG_LINE_MAX_LENGTH]; size_t length;
    assertFormat(LogFormatResult::SUCCESS, formatter.format(record, exact, needed + 1U, length)); TEST_ASSERT_EQUAL_UINT32(needed, length);
    char guarded[32]; memset(guarded, '?', sizeof(guarded));
    assertFormat(LogFormatResult::BUFFER_TOO_SMALL, formatter.format(record, guarded, 20U, length));
    TEST_ASSERT_EQUAL_INT8('\0', guarded[0]); TEST_ASSERT_EQUAL_INT8('?', guarded[20]); TEST_ASSERT_EQUAL_UINT32(0U, length);
    char one[2] = {'?', '!'}; assertFormat(LogFormatResult::BUFFER_TOO_SMALL, formatter.format(record, one, 1U, length));
    TEST_ASSERT_EQUAL_INT8('\0', one[0]); TEST_ASSERT_EQUAL_INT8('!', one[1]);
}

void test_writer_maps_output_results_and_calls_once()
{
    FakeTextOutput output; SerialLogWriter writer(output); LogRecord record = makeRecord();
    assertWrite(LogWriteResult::WRITTEN, writer.write(record)); TEST_ASSERT_EQUAL_UINT32(1U, output.calls);
    TEST_ASSERT_EQUAL_UINT32(strlen(output.lastData), output.lastLength); TEST_ASSERT_EQUAL_INT8('\n', output.lastData[output.lastLength - 1U]);
    output.result = TextOutputResult::RETRY_LATER; assertWrite(LogWriteResult::RETRY_LATER, writer.write(record)); TEST_ASSERT_EQUAL_UINT32(2U, output.calls);
    output.result = TextOutputResult::FAILED; assertWrite(LogWriteResult::FAILED, writer.write(record)); TEST_ASSERT_EQUAL_UINT32(3U, output.calls);
    output.result = TextOutputResult::COUNT; assertWrite(LogWriteResult::FAILED, writer.write(record));
    output.result = static_cast<TextOutputResult>(255U); assertWrite(LogWriteResult::FAILED, writer.write(record));
    const size_t before = output.calls; record.setMessage("bad|message");
    assertWrite(LogWriteResult::FAILED, writer.write(record)); TEST_ASSERT_EQUAL_UINT32(before, output.calls);
}

void test_logger_runtime_success_retry_failure_and_second_writer()
{
    LogQueue queue; LoggerRuntime runtime(queue); FakeTextOutput output; SerialLogWriter serial(output); FakeLogWriter second;
    runtime.addWriter(serial); runtime.addWriter(second); queue.publish(makeRecord());
    assertUpdate(LoggerUpdateResult::SUCCESS, runtime.update()); TEST_ASSERT_TRUE(queue.empty()); TEST_ASSERT_EQUAL_UINT32(1U, second.calls);
    output.result = TextOutputResult::RETRY_LATER; queue.publish(makeRecord());
    assertUpdate(LoggerUpdateResult::RETRY_LATER, runtime.update()); TEST_ASSERT_EQUAL_UINT32(1U, queue.size()); TEST_ASSERT_EQUAL_UINT32(2U, second.calls);
    output.result = TextOutputResult::SUCCESS; assertUpdate(LoggerUpdateResult::SUCCESS, runtime.update()); TEST_ASSERT_TRUE(queue.empty());
    output.result = TextOutputResult::FAILED; queue.publish(makeRecord());
    assertUpdate(LoggerUpdateResult::WRITER_FAILED, runtime.update()); TEST_ASSERT_TRUE(queue.empty()); TEST_ASSERT_EQUAL_UINT32(4U, second.calls);
}

void test_event_to_serial_pipeline_is_sequential_not_recursive()
{
    EventQueue eventQueue; EventDispatcher eventDispatcher(eventQueue); LogQueue logQueue;
    SequentialLogIdProvider ids; EventToLoggerAdapter adapter(logQueue, ids); eventDispatcher.addHandler(adapter);
    FakeTextOutput output; SerialLogWriter serial(output); LoggerRuntime logger(logQueue); logger.addWriter(serial);
    eventQueue.publish(makeEvent());
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(EventDispatchResult::SUCCESS), static_cast<uint8_t>(eventDispatcher.update()));
    TEST_ASSERT_EQUAL_UINT32(1U, logQueue.size()); TEST_ASSERT_EQUAL_UINT32(0U, output.calls);
    assertUpdate(LoggerUpdateResult::SUCCESS, logger.update()); TEST_ASSERT_TRUE(logQueue.empty()); TEST_ASSERT_EQUAL_UINT32(1U, output.calls);
    TEST_ASSERT_NOT_NULL(strstr(output.lastData, "|level=WARNING|cat=SECURITY|src=COMMAND|"));
    TEST_ASSERT_NOT_NULL(strstr(output.lastData, "|code=202|")); TEST_ASSERT_NOT_NULL(strstr(output.lastData, "|msg=command rejected\n"));
}

static_assert(std::is_base_of<LogWriter, SerialLogWriter>::value, "Writer باید LogWriter باشد");

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_formatter_exact_output_and_reuse);
    RUN_TEST(test_unsigned_and_signed_extremes);
    RUN_TEST(test_all_enum_names_and_invalid_enums);
    RUN_TEST(test_message_validation_boundary_and_utf8);
    RUN_TEST(test_invalid_arguments_records_and_failure_cleanup);
    RUN_TEST(test_buffer_boundaries_do_not_overwrite_guard);
    RUN_TEST(test_writer_maps_output_results_and_calls_once);
    RUN_TEST(test_logger_runtime_success_retry_failure_and_second_writer);
    RUN_TEST(test_event_to_serial_pipeline_is_sequential_not_recursive);
    UNITY_END();
}
void loop() {}
