#include <Arduino.h>
#include <EventDispatcher.h>
#include <type_traits>
#include <unity.h>

class EventQueueTestAccess
{
public:
    static void invalidateHead(EventQueue& queue) { queue.events_[queue.head_].id = INVALID_EVENT_ID; }
};

namespace
{
Event makeEvent(EventId id = 1U, EventType type = EventType::DEVICE_STATE_CHANGED)
{
    Event event;
    event.id = id;
    event.type = type;
    event.sourceType = EventSourceType::DEVICE;
    event.sourceId = 7U;
    event.timestampMs = 100U + id;
    event.payload.value1 = id * 10U;
    return event;
}

class FakeEventHandler : public EventHandler
{
public:
    EventHandleResult result = EventHandleResult::HANDLED;
    size_t calls = 0U;
    Event lastEvent{};
    size_t* sequence = nullptr;
    size_t calledAt = 0U;
    EventHandleResult handle(const Event& event) override
    {
        ++calls; lastEvent = event;
        if (sequence != nullptr) calledAt = ++(*sequence);
        return result;
    }
};

class PublishingEventHandler final : public FakeEventHandler
{
public:
    explicit PublishingEventHandler(EventSink& sink) : sink_(sink) {}
    Event eventToPublish = makeEvent(99U);
    EventPublishResult publishResult = EventPublishResult::INVALID_EVENT;
    EventHandleResult handle(const Event& event) override
    {
        const EventHandleResult handled = FakeEventHandler::handle(event);
        publishResult = sink_.publish(eventToPublish);
        return handled;
    }
private:
    EventSink& sink_;
};

class RecursiveEventHandler final : public EventHandler
{
public:
    explicit RecursiveEventHandler(EventDispatcher& dispatcher) : dispatcher_(dispatcher) {}
    EventDispatchResult recursiveResult = EventDispatchResult::SUCCESS;
    size_t calls = 0U;
    EventHandleResult handle(const Event&) override
    { ++calls; recursiveResult = dispatcher_.update(); return EventHandleResult::HANDLED; }
private:
    EventDispatcher& dispatcher_;
};

void assertPublish(EventPublishResult expected, EventPublishResult actual)
{ TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected), static_cast<uint8_t>(actual)); }
void assertDispatch(EventDispatchResult expected, EventDispatchResult actual)
{ TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected), static_cast<uint8_t>(actual)); }
}

void test_event_model_and_copy()
{
    Event event; TEST_ASSERT_FALSE(event.isValid());
    TEST_ASSERT_EQUAL_UINT32(0U, event.payload.value1); TEST_ASSERT_EQUAL_UINT32(0U, event.payload.value2);
    TEST_ASSERT_EQUAL_INT32(0, event.payload.signedValue); TEST_ASSERT_FALSE(event.payload.flag);
    event = makeEvent(); TEST_ASSERT_TRUE(event.isValid()); const Event original = event;
    TEST_ASSERT_TRUE(event.isValid()); TEST_ASSERT_EQUAL_UINT32(original.id, event.id);
    Event copy = event; event.payload.value1 = 500U; TEST_ASSERT_EQUAL_UINT32(10U, copy.payload.value1);
    event = makeEvent(); event.id = 0U; TEST_ASSERT_FALSE(event.isValid());
    event = makeEvent(); event.type = EventType::NONE; TEST_ASSERT_FALSE(event.isValid());
    event = makeEvent(); event.type = static_cast<EventType>(65000U); TEST_ASSERT_FALSE(event.isValid());
    event = makeEvent(); event.severity = static_cast<EventSeverity>(255U); TEST_ASSERT_FALSE(event.isValid());
    event = makeEvent(); event.sourceType = static_cast<EventSourceType>(255U); TEST_ASSERT_FALSE(event.isValid());
    event = makeEvent(); event.sourceId = 0U; TEST_ASSERT_FALSE(event.isValid());
    event = makeEvent(); event.sourceType = EventSourceType::SYSTEM; event.sourceId = 0U; TEST_ASSERT_TRUE(event.isValid());
}

void test_queue_validation_duplicate_full_and_copy()
{
    EventQueue queue; TEST_ASSERT_TRUE(queue.empty()); TEST_ASSERT_FALSE(queue.full());
    TEST_ASSERT_EQUAL_UINT32(EVENT_QUEUE_CAPACITY, queue.capacity()); TEST_ASSERT_NULL(queue.peek());
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(EventQueueResult::QUEUE_EMPTY), static_cast<uint8_t>(queue.consume()));
    Event event = makeEvent(); assertPublish(EventPublishResult::SUCCESS, queue.publish(event));
    assertPublish(EventPublishResult::DUPLICATE_EVENT, queue.publish(event)); TEST_ASSERT_EQUAL_UINT32(1U, queue.size());
    Event invalid; assertPublish(EventPublishResult::INVALID_EVENT, queue.publish(invalid)); TEST_ASSERT_EQUAL_UINT32(1U, queue.size());
    event.payload.value1 = 900U; TEST_ASSERT_EQUAL_UINT32(10U, queue.peek()->payload.value1);
    for (EventId id = 2U; id <= EVENT_QUEUE_CAPACITY; ++id) assertPublish(EventPublishResult::SUCCESS, queue.publish(makeEvent(id)));
    TEST_ASSERT_TRUE(queue.full()); const size_t before = queue.size();
    assertPublish(EventPublishResult::QUEUE_FULL, queue.publish(makeEvent(100U))); TEST_ASSERT_EQUAL_UINT32(before, queue.size());
}

void test_queue_fifo_wraparound_clear()
{
    EventQueue queue;
    for (EventId id = 1U; id <= EVENT_QUEUE_CAPACITY; ++id) queue.publish(makeEvent(id));
    for (EventId id = 1U; id <= 8U; ++id) { TEST_ASSERT_EQUAL_UINT32(id, queue.peek()->id); queue.consume(); }
    for (EventId id = EVENT_QUEUE_CAPACITY + 1U; id <= EVENT_QUEUE_CAPACITY + 8U; ++id) queue.publish(makeEvent(id));
    for (EventId id = 9U; id <= EVENT_QUEUE_CAPACITY + 8U; ++id) { TEST_ASSERT_EQUAL_UINT32(id, queue.peek()->id); queue.consume(); }
    TEST_ASSERT_TRUE(queue.empty()); queue.publish(makeEvent(50U)); queue.clear();
    TEST_ASSERT_TRUE(queue.empty()); TEST_ASSERT_NULL(queue.peek()); queue.publish(makeEvent(51U)); TEST_ASSERT_EQUAL_UINT32(51U, queue.peek()->id);
}

void test_handler_registration_and_order()
{
    EventQueue queue; EventDispatcher dispatcher(queue); FakeEventHandler handlers[EVENT_HANDLER_CAPACITY + 1U];
    TEST_ASSERT_EQUAL_UINT32(EVENT_HANDLER_CAPACITY, dispatcher.handlerCapacity());
    for (size_t index = 0U; index < EVENT_HANDLER_CAPACITY; ++index)
        TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(dispatcher.addHandler(handlers[index])));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(EventHandlerRegistrationResult::DUPLICATE_HANDLER), static_cast<uint8_t>(dispatcher.addHandler(handlers[0])));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(EventHandlerRegistrationResult::REGISTRY_FULL), static_cast<uint8_t>(dispatcher.addHandler(handlers[EVENT_HANDLER_CAPACITY])));
    TEST_ASSERT_EQUAL_UINT32(EVENT_HANDLER_CAPACITY, dispatcher.handlerCount());
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(dispatcher.removeHandler(handlers[2])));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(EventHandlerRegistrationResult::HANDLER_NOT_FOUND), static_cast<uint8_t>(dispatcher.removeHandler(handlers[2])));
    size_t sequence = 0U; for (size_t index = 0U; index < EVENT_HANDLER_CAPACITY; ++index) handlers[index].sequence = &sequence;
    queue.publish(makeEvent()); assertDispatch(EventDispatchResult::SUCCESS, dispatcher.update());
    TEST_ASSERT_EQUAL_UINT32(0U, handlers[2].calls); TEST_ASSERT_EQUAL_UINT32(3U, handlers[3].calledAt);
    dispatcher.removeHandler(handlers[0]); dispatcher.addHandler(handlers[0]); queue.publish(makeEvent(2U)); dispatcher.update();
    TEST_ASSERT_EQUAL_UINT32(sequence, handlers[0].calledAt);
}

void test_dispatch_results_consumption_and_next_event()
{
    EventQueue queue; EventDispatcher dispatcher(queue); FakeEventHandler first; FakeEventHandler second;
    assertDispatch(EventDispatchResult::QUEUE_EMPTY, dispatcher.update());
    queue.publish(makeEvent()); assertDispatch(EventDispatchResult::NO_HANDLERS, dispatcher.update()); TEST_ASSERT_TRUE(queue.empty());
    dispatcher.addHandler(first); dispatcher.addHandler(second); second.result = EventHandleResult::IGNORED;
    queue.publish(makeEvent(2U)); queue.publish(makeEvent(3U)); const Event snapshot = *queue.peek();
    assertDispatch(EventDispatchResult::SUCCESS, dispatcher.update()); TEST_ASSERT_EQUAL_UINT32(1U, first.calls);
    TEST_ASSERT_EQUAL_UINT32(snapshot.id, first.lastEvent.id); TEST_ASSERT_EQUAL_UINT32(3U, queue.peek()->id);
    first.result = EventHandleResult::IGNORED; assertDispatch(EventDispatchResult::EVENT_IGNORED, dispatcher.update()); TEST_ASSERT_TRUE(queue.empty());
}

void test_failure_invalid_result_and_no_retry()
{
    EventQueue queue; EventDispatcher dispatcher(queue); FakeEventHandler failed; FakeEventHandler later;
    failed.result = EventHandleResult::FAILED; dispatcher.addHandler(failed); dispatcher.addHandler(later);
    queue.publish(makeEvent()); assertDispatch(EventDispatchResult::HANDLER_FAILED, dispatcher.update());
    TEST_ASSERT_EQUAL_UINT32(1U, failed.calls); TEST_ASSERT_EQUAL_UINT32(1U, later.calls); TEST_ASSERT_TRUE(queue.empty());
    queue.publish(makeEvent(2U)); failed.result = static_cast<EventHandleResult>(255U);
    assertDispatch(EventDispatchResult::HANDLER_FAILED, dispatcher.update()); TEST_ASSERT_EQUAL_UINT32(2U, later.calls);
    assertDispatch(EventDispatchResult::QUEUE_EMPTY, dispatcher.update()); TEST_ASSERT_EQUAL_UINT32(2U, failed.calls);
}

void test_corrupt_event_is_dropped_and_queue_progresses()
{
    EventQueue queue; EventDispatcher dispatcher(queue); FakeEventHandler handler; dispatcher.addHandler(handler);
    queue.publish(makeEvent()); queue.publish(makeEvent(2U)); EventQueueTestAccess::invalidateHead(queue);
    assertDispatch(EventDispatchResult::INVALID_EVENT, dispatcher.update()); TEST_ASSERT_EQUAL_UINT32(0U, handler.calls);
    TEST_ASSERT_EQUAL_UINT32(2U, queue.peek()->id); assertDispatch(EventDispatchResult::SUCCESS, dispatcher.update());
}

void test_reentrant_publish_is_deferred_and_full_is_atomic()
{
    EventQueue queue; EventDispatcher dispatcher(queue); PublishingEventHandler publisher(queue); dispatcher.addHandler(publisher);
    queue.publish(makeEvent()); assertDispatch(EventDispatchResult::SUCCESS, dispatcher.update());
    assertPublish(EventPublishResult::SUCCESS, publisher.publishResult); TEST_ASSERT_EQUAL_UINT32(0U, publisher.lastEvent.id - 1U);
    TEST_ASSERT_EQUAL_UINT32(99U, queue.peek()->id); publisher.eventToPublish = makeEvent(100U);
    assertDispatch(EventDispatchResult::SUCCESS, dispatcher.update()); TEST_ASSERT_EQUAL_UINT32(100U, queue.peek()->id);
    queue.clear(); for (EventId id = 1U; id <= EVENT_QUEUE_CAPACITY; ++id) queue.publish(makeEvent(id));
    publisher.eventToPublish = makeEvent(500U); dispatcher.update(); assertPublish(EventPublishResult::QUEUE_FULL, publisher.publishResult);
    TEST_ASSERT_EQUAL_UINT32(EVENT_QUEUE_CAPACITY - 1U, queue.size());
}

void test_recursive_dispatch_is_rejected_without_mutation()
{
    EventQueue queue; EventDispatcher dispatcher(queue); RecursiveEventHandler handler(dispatcher);
    dispatcher.addHandler(handler); queue.publish(makeEvent());
    assertDispatch(EventDispatchResult::SUCCESS, dispatcher.update());
    assertDispatch(EventDispatchResult::REENTRANT_CALL, handler.recursiveResult);
    TEST_ASSERT_EQUAL_UINT32(1U, handler.calls); TEST_ASSERT_TRUE(queue.empty());
}

static_assert(std::is_base_of<EventSink, EventQueue>::value, "Producer باید فقط EventSink را ببیند");
static_assert(std::is_same<decltype(static_cast<const EventQueue&>(*static_cast<EventQueue*>(nullptr)).peek()), const Event*>::value, "peek باید فقط const Event بدهد");
static_assert(std::is_trivially_copyable<EventPayload>::value, "Payload باید Copy ساده داشته باشد");

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_event_model_and_copy); RUN_TEST(test_queue_validation_duplicate_full_and_copy);
    RUN_TEST(test_queue_fifo_wraparound_clear); RUN_TEST(test_handler_registration_and_order);
    RUN_TEST(test_dispatch_results_consumption_and_next_event); RUN_TEST(test_failure_invalid_result_and_no_retry);
    RUN_TEST(test_corrupt_event_is_dropped_and_queue_progresses); RUN_TEST(test_reentrant_publish_is_deferred_and_full_is_atomic);
    RUN_TEST(test_recursive_dispatch_is_rejected_without_mutation);
    UNITY_END();
}
void loop() {}
