#include "EventToNotificationDispatchBinding.h"

EventToNotificationDispatchBinding::EventToNotificationDispatchBinding(
    EventQueue& queue,
    EventDispatcher& dispatcher,
    EventToNotificationBridge& bridge) :
    queue_(queue),
    dispatcher_(dispatcher),
    bridge_(bridge),
    currentDispatchNow_(0U),
    initialized_(false),
    dispatchActive_(false) {}

RuntimeIntegrationResult EventToNotificationDispatchBinding::begin()
{
    if (dispatchActive_) return RuntimeIntegrationResult::REJECTED;
    if (initialized_) return RuntimeIntegrationResult::SUCCESS;
    if (dispatcher_.addHandler(*this) != EventHandlerRegistrationResult::SUCCESS)
        return RuntimeIntegrationResult::FAILED;
    initialized_ = true;
    return RuntimeIntegrationResult::SUCCESS;
}

RuntimeIntegrationResult EventToNotificationDispatchBinding::beginDispatch(
    NotificationTimestamp now)
{
    if (!initialized_) return RuntimeIntegrationResult::NOT_INITIALIZED;
    if (dispatchActive_) return RuntimeIntegrationResult::REJECTED;
    currentDispatchNow_ = now;
    dispatchActive_ = true;
    return RuntimeIntegrationResult::SUCCESS;
}

RuntimeIntegrationResult EventToNotificationDispatchBinding::endDispatch()
{
    if (!dispatchActive_) return RuntimeIntegrationResult::INVALID_ARGUMENT;
    currentDispatchNow_ = 0U;
    dispatchActive_ = false;
    return RuntimeIntegrationResult::SUCCESS;
}

EventHandleResult EventToNotificationDispatchBinding::mapHandlerResult(
    RuntimeIntegrationResult result)
{
    switch (result)
    {
        case RuntimeIntegrationResult::SUCCESS:
        case RuntimeIntegrationResult::ACCEPTED:
        case RuntimeIntegrationResult::IN_PROGRESS:
            return EventHandleResult::HANDLED;
        case RuntimeIntegrationResult::NO_CHANGE:
        case RuntimeIntegrationResult::IGNORED:
        case RuntimeIntegrationResult::FILTERED:
        case RuntimeIntegrationResult::MAPPING_DISABLED:
            return EventHandleResult::IGNORED;
        case RuntimeIntegrationResult::RETRY_LATER:
        case RuntimeIntegrationResult::REJECTED:
        case RuntimeIntegrationResult::FAILED:
        case RuntimeIntegrationResult::INTERNAL_ERROR:
        case RuntimeIntegrationResult::NOT_INITIALIZED:
        case RuntimeIntegrationResult::INVALID_ARGUMENT:
        case RuntimeIntegrationResult::INVALID_INPUT:
        case RuntimeIntegrationResult::MAPPING_NOT_FOUND:
        case RuntimeIntegrationResult::REGISTRY_FULL:
        case RuntimeIntegrationResult::REGISTRY_LOCKED:
        case RuntimeIntegrationResult::DUPLICATE_MAPPING:
        case RuntimeIntegrationResult::COUNT:
        default:
            return EventHandleResult::FAILED;
    }
}

EventHandleResult EventToNotificationDispatchBinding::handle(const Event& event)
{
    if (!dispatchActive_) return EventHandleResult::FAILED;
    return mapHandlerResult(bridge_.onEvent(event, currentDispatchNow_));
}

RuntimeIntegrationResult EventToNotificationDispatchBinding::mapDispatchResult(
    EventDispatchResult result)
{
    switch (result)
    {
        case EventDispatchResult::SUCCESS: return RuntimeIntegrationResult::SUCCESS;
        case EventDispatchResult::QUEUE_EMPTY: return RuntimeIntegrationResult::NO_CHANGE;
        case EventDispatchResult::EVENT_IGNORED: return RuntimeIntegrationResult::IGNORED;
        case EventDispatchResult::NO_HANDLERS:
        case EventDispatchResult::HANDLER_FAILED:
        case EventDispatchResult::INVALID_EVENT:
            return RuntimeIntegrationResult::FAILED;
        case EventDispatchResult::REENTRANT_CALL: return RuntimeIntegrationResult::REJECTED;
        case EventDispatchResult::INTERNAL_ERROR:
        default:
            return RuntimeIntegrationResult::INTERNAL_ERROR;
    }
}

RuntimeIntegrationResult EventToNotificationDispatchBinding::combineDispatchResults(
    RuntimeIntegrationResult current,
    RuntimeIntegrationResult candidate)
{
    if (current == RuntimeIntegrationResult::INTERNAL_ERROR ||
        candidate == RuntimeIntegrationResult::INTERNAL_ERROR)
        return RuntimeIntegrationResult::INTERNAL_ERROR;
    if (current == RuntimeIntegrationResult::FAILED ||
        candidate == RuntimeIntegrationResult::FAILED)
        return RuntimeIntegrationResult::FAILED;
    if (current == RuntimeIntegrationResult::REJECTED ||
        candidate == RuntimeIntegrationResult::REJECTED)
        return RuntimeIntegrationResult::REJECTED;
    if (current == RuntimeIntegrationResult::SUCCESS ||
        candidate == RuntimeIntegrationResult::SUCCESS)
        return RuntimeIntegrationResult::SUCCESS;
    if (current == RuntimeIntegrationResult::IGNORED ||
        candidate == RuntimeIntegrationResult::IGNORED)
        return RuntimeIntegrationResult::IGNORED;
    return candidate;
}

RuntimeIntegrationResult EventToNotificationDispatchBinding::dispatchEvents(
    NotificationTimestamp now)
{
    const RuntimeIntegrationResult beginResult = beginDispatch(now);
    if (beginResult != RuntimeIntegrationResult::SUCCESS) return beginResult;

    RuntimeIntegrationResult overall = RuntimeIntegrationResult::NO_CHANGE;
    const size_t eventCount = queue_.size();
    for (size_t index = 0U; index < eventCount; ++index)
    {
        const EventDispatchResult dispatchResult = dispatcher_.update();
        if (dispatchResult == EventDispatchResult::QUEUE_EMPTY) break;
        const RuntimeIntegrationResult mapped = mapDispatchResult(dispatchResult);
        overall = combineDispatchResults(overall, mapped);
        if (dispatchResult == EventDispatchResult::INTERNAL_ERROR ||
            dispatchResult == EventDispatchResult::REENTRANT_CALL)
            break;
    }

    const RuntimeIntegrationResult endResult = endDispatch();
    return endResult == RuntimeIntegrationResult::SUCCESS ? overall : endResult;
}

bool EventToNotificationDispatchBinding::isInitialized() const { return initialized_; }
bool EventToNotificationDispatchBinding::isDispatching() const { return dispatchActive_; }
