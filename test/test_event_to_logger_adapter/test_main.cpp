#include <Arduino.h>
#include <EventDispatcher.h>
#include <EventToLoggerAdapter.h>
#include <LogQueue.h>
#include <SequentialLogIdProvider.h>
#include <type_traits>
#include <unity.h>

namespace
{
Event makeEvent(EventId id = 1U, EventSourceType source = EventSourceType::DEVICE)
{
    Event event;
    event.id = id;
    event.type = EventType::DEVICE_STATE_CHANGED;
    event.severity = EventSeverity::INFO;
    event.sourceType = source;
    event.sourceId = source == EventSourceType::SYSTEM ? 0U : 7U;
    event.timestampMs = 100U + id;
    event.payload.value1 = 11U;
    event.payload.value2 = 22U;
    event.payload.signedValue = -3;
    event.payload.flag = true;
    event.setMessage("event message");
    return event;
}

class FakeLogSink final : public LogSink
{
public:
    LogPublishResult result = LogPublishResult::SUCCESS;
    size_t calls = 0U;
    LogRecord lastRecord{};
    size_t* sequence = nullptr;
    size_t calledAt = 0U;
    LogPublishResult publish(const LogRecord& record) override
    {
        ++calls; lastRecord = record;
        if (sequence != nullptr) calledAt = ++(*sequence);
        return result;
    }
};

class FakeLogIdProvider final : public LogIdProvider
{
public:
    LogId nextId = 50U;
    size_t calls = 0U;
    LogId nextLogId() override { ++calls; return nextId; }
};

class SequencedEventHandler final : public EventHandler
{
public:
    EventHandleResult result = EventHandleResult::HANDLED;
    size_t calls = 0U;
    size_t* sequence = nullptr;
    size_t calledAt = 0U;
    EventHandleResult handle(const Event&) override
    {
        ++calls;
        if (sequence != nullptr) calledAt = ++(*sequence);
        return result;
    }
};

void assertHandle(EventHandleResult expected, EventHandleResult actual)
{ TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected), static_cast<uint8_t>(actual)); }
}

void test_valid_event_maps_all_fields_and_publishes_once()
{
    FakeLogSink sink; FakeLogIdProvider ids; EventToLoggerAdapter adapter(sink, ids); Event event = makeEvent();
    assertHandle(EventHandleResult::HANDLED, adapter.handle(event));
    TEST_ASSERT_EQUAL_UINT32(1U, ids.calls); TEST_ASSERT_EQUAL_UINT32(1U, sink.calls);
    const LogRecord& record = sink.lastRecord; TEST_ASSERT_TRUE(record.isValid());
    TEST_ASSERT_EQUAL_UINT32(50U, record.id); TEST_ASSERT_EQUAL_UINT32(event.timestampMs, record.timestampMs);
    TEST_ASSERT_EQUAL_UINT32(event.sourceId, record.sourceId);
    TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(event.type), record.code);
    TEST_ASSERT_EQUAL_UINT32(11U, record.payload.value1); TEST_ASSERT_EQUAL_UINT32(22U, record.payload.value2);
    TEST_ASSERT_EQUAL_INT32(-3, record.payload.signedValue); TEST_ASSERT_TRUE(record.payload.flag);
    TEST_ASSERT_EQUAL_STRING("event message", record.message);
    event.id = 999U; TEST_ASSERT_EQUAL_UINT32(50U, sink.lastRecord.id);
    event = makeEvent(2U); event.message[0] = '\0'; assertHandle(EventHandleResult::HANDLED, adapter.handle(event));
    TEST_ASSERT_EQUAL_STRING("", sink.lastRecord.message);
}

void test_severity_mapping_and_invalid_values_fail_before_dependencies()
{
    const EventSeverity severities[] = { EventSeverity::INFO, EventSeverity::NOTICE,
        EventSeverity::WARNING, EventSeverity::ERROR, EventSeverity::CRITICAL };
    const LogLevel levels[] = { LogLevel::INFO, LogLevel::NOTICE, LogLevel::WARNING,
        LogLevel::ERROR, LogLevel::CRITICAL };
    for (size_t index = 0U; index < 5U; ++index)
    {
        FakeLogSink sink; FakeLogIdProvider ids; EventToLoggerAdapter adapter(sink, ids);
        Event event = makeEvent(); event.severity = severities[index];
        assertHandle(EventHandleResult::HANDLED, adapter.handle(event));
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(levels[index]), static_cast<uint8_t>(sink.lastRecord.level));
    }
    FakeLogSink sink; FakeLogIdProvider ids; EventToLoggerAdapter adapter(sink, ids); Event invalid = makeEvent();
    invalid.severity = EventSeverity::COUNT; assertHandle(EventHandleResult::FAILED, adapter.handle(invalid));
    invalid = makeEvent(); invalid.severity = static_cast<EventSeverity>(255U);
    assertHandle(EventHandleResult::FAILED, adapter.handle(invalid));
    TEST_ASSERT_EQUAL_UINT32(0U, ids.calls); TEST_ASSERT_EQUAL_UINT32(0U, sink.calls);
}

void test_source_and_category_mapping_is_exhaustive()
{
    const EventSourceType eventSources[] = { EventSourceType::SYSTEM, EventSourceType::COMMAND,
        EventSourceType::DEVICE, EventSourceType::ENDPOINT, EventSourceType::NODE,
        EventSourceType::TRANSPORT, EventSourceType::STORAGE, EventSourceType::RULE,
        EventSourceType::SCENE, EventSourceType::SCHEDULER, EventSourceType::USER };
    const LogSourceType logSources[] = { LogSourceType::SYSTEM, LogSourceType::COMMAND,
        LogSourceType::DEVICE, LogSourceType::ENDPOINT, LogSourceType::NODE,
        LogSourceType::TRANSPORT, LogSourceType::STORAGE, LogSourceType::RULE,
        LogSourceType::SCENE, LogSourceType::SCHEDULER, LogSourceType::USER };
    const LogCategory categories[] = { LogCategory::SYSTEM, LogCategory::COMMAND,
        LogCategory::DEVICE, LogCategory::DEVICE, LogCategory::NODE,
        LogCategory::TRANSPORT, LogCategory::STORAGE, LogCategory::AUTOMATION,
        LogCategory::AUTOMATION, LogCategory::AUTOMATION, LogCategory::DIAGNOSTIC };
    for (size_t index = 0U; index < 11U; ++index)
    {
        FakeLogSink sink; FakeLogIdProvider ids; EventToLoggerAdapter adapter(sink, ids);
        Event event = makeEvent(static_cast<EventId>(index + 1U), eventSources[index]);
        assertHandle(EventHandleResult::HANDLED, adapter.handle(event));
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(logSources[index]), static_cast<uint8_t>(sink.lastRecord.sourceType));
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(categories[index]), static_cast<uint8_t>(sink.lastRecord.category));
        TEST_ASSERT_EQUAL_UINT32(event.sourceId, sink.lastRecord.sourceId);
    }
}

void test_security_event_types_override_source_category()
{
    const EventType types[] = { EventType::COMMAND_REJECTED,
        EventType::SAFETY_INTERLOCK_TRIGGERED, EventType::CONFIRMATION_REQUIRED };
    for (size_t index = 0U; index < 3U; ++index)
    {
        FakeLogSink sink; FakeLogIdProvider ids; EventToLoggerAdapter adapter(sink, ids);
        Event event = makeEvent(); event.type = types[index]; event.sourceType = EventSourceType::DEVICE;
        assertHandle(EventHandleResult::HANDLED, adapter.handle(event));
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(LogCategory::SECURITY), static_cast<uint8_t>(sink.lastRecord.category));
    }
}

void test_invalid_event_never_requests_id_or_publishes()
{
    FakeLogSink sink; FakeLogIdProvider ids; EventToLoggerAdapter adapter(sink, ids);
    Event events[7];
    events[0] = Event{};
    events[1] = makeEvent(); events[1].id = INVALID_EVENT_ID;
    events[2] = makeEvent(); events[2].type = static_cast<EventType>(65000U);
    events[3] = makeEvent(); events[3].status = static_cast<EventStatus>(255U);
    events[4] = makeEvent(); events[4].sourceType = EventSourceType::COUNT;
    events[5] = makeEvent(); events[5].sourceId = 0U;
    events[6] = makeEvent(); memset(events[6].message, 'x', sizeof(events[6].message));
    for (size_t index = 0U; index < 7U; ++index)
        assertHandle(EventHandleResult::FAILED, adapter.handle(events[index]));
    TEST_ASSERT_EQUAL_UINT32(0U, ids.calls); TEST_ASSERT_EQUAL_UINT32(0U, sink.calls);
}

void test_log_id_provider_zero_wrap_and_no_retry()
{
    SequentialLogIdProvider defaultIds; TEST_ASSERT_EQUAL_UINT32(1U, defaultIds.nextLogId());
    SequentialLogIdProvider customIds(40U); TEST_ASSERT_EQUAL_UINT32(40U, customIds.nextLogId());
    SequentialLogIdProvider normalized(INVALID_LOG_ID); TEST_ASSERT_EQUAL_UINT32(1U, normalized.nextLogId());
    SequentialLogIdProvider wrapping(UINT32_MAX); TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, wrapping.nextLogId());
    TEST_ASSERT_EQUAL_UINT32(1U, wrapping.nextLogId());

    FakeLogSink sink; FakeLogIdProvider invalid; invalid.nextId = INVALID_LOG_ID;
    EventToLoggerAdapter adapter(sink, invalid); assertHandle(EventHandleResult::FAILED, adapter.handle(makeEvent()));
    TEST_ASSERT_EQUAL_UINT32(1U, invalid.calls); TEST_ASSERT_EQUAL_UINT32(0U, sink.calls);
}

void test_publish_results_map_fail_closed_once_and_allow_id_gaps()
{
    const LogPublishResult failures[] = { LogPublishResult::INVALID_RECORD,
        LogPublishResult::QUEUE_FULL, LogPublishResult::DUPLICATE_LOG_ID,
        LogPublishResult::COUNT, static_cast<LogPublishResult>(255U) };
    for (size_t index = 0U; index < 5U; ++index)
    {
        FakeLogSink sink; FakeLogIdProvider ids; sink.result = failures[index];
        EventToLoggerAdapter adapter(sink, ids); assertHandle(EventHandleResult::FAILED, adapter.handle(makeEvent()));
        TEST_ASSERT_EQUAL_UINT32(1U, ids.calls); TEST_ASSERT_EQUAL_UINT32(1U, sink.calls);
    }
    FakeLogSink sink; SequentialLogIdProvider ids; EventToLoggerAdapter adapter(sink, ids);
    sink.result = LogPublishResult::QUEUE_FULL; adapter.handle(makeEvent()); TEST_ASSERT_EQUAL_UINT32(1U, sink.lastRecord.id);
    sink.result = LogPublishResult::SUCCESS; adapter.handle(makeEvent(2U)); TEST_ASSERT_EQUAL_UINT32(2U, sink.lastRecord.id);
}

void test_message_boundary_is_copied_without_truncation()
{
    FakeLogSink sink; FakeLogIdProvider ids; EventToLoggerAdapter adapter(sink, ids); Event event = makeEvent();
    for (size_t index = 0U; index < EVENT_MESSAGE_MAX_LENGTH - 1U; ++index) event.message[index] = 'a';
    event.message[EVENT_MESSAGE_MAX_LENGTH - 1U] = '\0';
    assertHandle(EventHandleResult::HANDLED, adapter.handle(event));
    TEST_ASSERT_EQUAL_UINT32(EVENT_MESSAGE_MAX_LENGTH - 1U, strlen(sink.lastRecord.message));
}

void test_real_log_queue_success_full_duplicate_and_atomic_size()
{
    LogQueue queue; SequentialLogIdProvider ids(10U); EventToLoggerAdapter adapter(queue, ids);
    Event event = makeEvent(); assertHandle(EventHandleResult::HANDLED, adapter.handle(event));
    TEST_ASSERT_EQUAL_UINT32(1U, queue.size()); TEST_ASSERT_EQUAL_UINT32(10U, queue.peek()->id);
    TEST_ASSERT_EQUAL_UINT32(event.timestampMs, queue.peek()->timestampMs);
    for (LogId index = 1U; index < LOG_QUEUE_CAPACITY; ++index) adapter.handle(makeEvent(index + 1U));
    const size_t before = queue.size(); assertHandle(EventHandleResult::FAILED, adapter.handle(makeEvent(100U)));
    TEST_ASSERT_EQUAL_UINT32(before, queue.size());

    LogQueue duplicateQueue; FakeLogIdProvider duplicateIds; duplicateIds.nextId = 77U;
    EventToLoggerAdapter duplicateAdapter(duplicateQueue, duplicateIds);
    assertHandle(EventHandleResult::HANDLED, duplicateAdapter.handle(makeEvent()));
    assertHandle(EventHandleResult::FAILED, duplicateAdapter.handle(makeEvent(2U)));
    TEST_ASSERT_EQUAL_UINT32(1U, duplicateQueue.size()); TEST_ASSERT_EQUAL_UINT32(2U, duplicateIds.calls);
}

void test_event_dispatcher_integration_preserves_order_and_no_recursive_logger()
{
    EventQueue eventQueue; EventDispatcher dispatcher(eventQueue); LogQueue logQueue;
    SequentialLogIdProvider ids; EventToLoggerAdapter adapter(logQueue, ids); dispatcher.addHandler(adapter);
    eventQueue.publish(makeEvent()); eventQueue.publish(makeEvent(2U));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(EventDispatchResult::SUCCESS), static_cast<uint8_t>(dispatcher.update()));
    TEST_ASSERT_EQUAL_UINT32(1U, eventQueue.size()); TEST_ASSERT_EQUAL_UINT32(1U, logQueue.size());
    TEST_ASSERT_EQUAL_UINT32(1U, logQueue.peek()->id); logQueue.consume();
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(EventDispatchResult::SUCCESS), static_cast<uint8_t>(dispatcher.update()));
    TEST_ASSERT_TRUE(eventQueue.empty()); TEST_ASSERT_EQUAL_UINT32(2U, logQueue.peek()->id);
}

void test_multiple_handlers_continue_in_registration_order_on_adapter_failure()
{
    EventQueue queue; EventDispatcher dispatcher(queue); FakeLogSink sink; FakeLogIdProvider ids;
    EventToLoggerAdapter adapter(sink, ids); SequencedEventHandler before; SequencedEventHandler after;
    size_t sequence = 0U; before.sequence = &sequence; sink.sequence = &sequence; after.sequence = &sequence;
    dispatcher.addHandler(before); dispatcher.addHandler(adapter); dispatcher.addHandler(after);
    queue.publish(makeEvent()); TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(EventDispatchResult::SUCCESS), static_cast<uint8_t>(dispatcher.update()));
    TEST_ASSERT_EQUAL_UINT32(1U, before.calledAt); TEST_ASSERT_EQUAL_UINT32(2U, sink.calledAt); TEST_ASSERT_EQUAL_UINT32(3U, after.calledAt);
    sink.result = LogPublishResult::QUEUE_FULL; sequence = 0U; queue.publish(makeEvent(2U));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(EventDispatchResult::HANDLER_FAILED), static_cast<uint8_t>(dispatcher.update()));
    TEST_ASSERT_EQUAL_UINT32(2U, after.calls); TEST_ASSERT_TRUE(queue.empty());
}

static_assert(std::is_base_of<EventHandler, EventToLoggerAdapter>::value, "Adapter باید EventHandler باشد");
static_assert(!std::is_copy_constructible<EventToLoggerAdapter>::value || sizeof(EventToLoggerAdapter) > 0U,
    "Adapter فقط Reference dependency نگه می‌دارد");

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_valid_event_maps_all_fields_and_publishes_once);
    RUN_TEST(test_severity_mapping_and_invalid_values_fail_before_dependencies);
    RUN_TEST(test_source_and_category_mapping_is_exhaustive);
    RUN_TEST(test_security_event_types_override_source_category);
    RUN_TEST(test_invalid_event_never_requests_id_or_publishes);
    RUN_TEST(test_log_id_provider_zero_wrap_and_no_retry);
    RUN_TEST(test_publish_results_map_fail_closed_once_and_allow_id_gaps);
    RUN_TEST(test_message_boundary_is_copied_without_truncation);
    RUN_TEST(test_real_log_queue_success_full_duplicate_and_atomic_size);
    RUN_TEST(test_event_dispatcher_integration_preserves_order_and_no_recursive_logger);
    RUN_TEST(test_multiple_handlers_continue_in_registration_order_on_adapter_failure);
    UNITY_END();
}
void loop() {}
