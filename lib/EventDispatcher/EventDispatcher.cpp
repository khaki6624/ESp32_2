#include "EventDispatcher.h"
EventDispatcher::EventDispatcher(EventQueue& queue) :
    queue_(queue), handlers_{}, handlerCount_(0U), dispatching_(false) {}
EventHandlerRegistrationResult EventDispatcher::addHandler(EventHandler& handler)
{
    for (size_t index = 0U; index < handlerCount_; ++index)
        if (handlers_[index] == &handler) return EventHandlerRegistrationResult::DUPLICATE_HANDLER;
    if (handlerCount_ == EVENT_HANDLER_CAPACITY) return EventHandlerRegistrationResult::REGISTRY_FULL;
    handlers_[handlerCount_++] = &handler;
    return EventHandlerRegistrationResult::SUCCESS;
}
EventHandlerRegistrationResult EventDispatcher::removeHandler(EventHandler& handler)
{
    for (size_t index = 0U; index < handlerCount_; ++index)
    {
        if (handlers_[index] != &handler) continue;
        for (size_t shift = index + 1U; shift < handlerCount_; ++shift)
            handlers_[shift - 1U] = handlers_[shift];
        --handlerCount_;
        handlers_[handlerCount_] = nullptr;
        return EventHandlerRegistrationResult::SUCCESS;
    }
    return EventHandlerRegistrationResult::HANDLER_NOT_FOUND;
}
EventDispatchResult EventDispatcher::update()
{
    if (dispatching_) return EventDispatchResult::REENTRANT_CALL;
    const Event* event = queue_.peek();
    if (event == nullptr) return EventDispatchResult::QUEUE_EMPTY;
    if (!event->isValid())
        return queue_.consume() == EventQueueResult::SUCCESS
            ? EventDispatchResult::INVALID_EVENT : EventDispatchResult::INTERNAL_ERROR;
    bool handled = false;
    bool failed = false;
    dispatching_ = true;
    for (size_t index = 0U; index < handlerCount_; ++index)
    {
        EventHandler* const handler = handlers_[index];
        if (handler == nullptr) { failed = true; continue; }
        const EventHandleResult result = handler->handle(*event);
        if (!isValidEventHandleResult(result) || result == EventHandleResult::FAILED) failed = true;
        else if (result == EventHandleResult::HANDLED) handled = true;
    }
    dispatching_ = false;
    if (queue_.consume() != EventQueueResult::SUCCESS) return EventDispatchResult::INTERNAL_ERROR;
    if (failed) return EventDispatchResult::HANDLER_FAILED;
    if (handlerCount_ == 0U) return EventDispatchResult::NO_HANDLERS;
    return handled ? EventDispatchResult::SUCCESS : EventDispatchResult::EVENT_IGNORED;
}
size_t EventDispatcher::handlerCount() const { return handlerCount_; }
size_t EventDispatcher::handlerCapacity() const { return EVENT_HANDLER_CAPACITY; }
