#include "EventBus.h"

namespace { constexpr size_t INVALID_EVENT_INDEX = EVENT_BUS_CAPACITY; }

EventBus::EventBus() : events_{}, count_(0) {}
void EventBus::clear() { for (size_t i=0;i<EVENT_BUS_CAPACITY;++i) events_[i]=Event{}; count_=0; }

EventResultCode EventBus::publish(const Event& event)
{
    if (!event.isValid()) return EventResultCode::INVALID_EVENT;
    if (event.status != EventStatus::PENDING) return EventResultCode::INVALID_STATUS;
    if (isFull()) return EventResultCode::QUEUE_FULL;
    if (contains(event.id)) return EventResultCode::DUPLICATE_ID;
    events_[count_++] = event;
    return EventResultCode::SUCCESS;
}

size_t EventBus::findIndexById(EventId id) const
{
    if (id==INVALID_EVENT_ID) return INVALID_EVENT_INDEX;
    for(size_t i=0;i<count_;++i) if(events_[i].id==id) return i;
    return INVALID_EVENT_INDEX;
}
Event* EventBus::findById(EventId id) { size_t i=findIndexById(id); return i==INVALID_EVENT_INDEX?nullptr:&events_[i]; }
const Event* EventBus::findById(EventId id) const { size_t i=findIndexById(id); return i==INVALID_EVENT_INDEX?nullptr:&events_[i]; }
Event* EventBus::peek() { return getAt(0); }
const Event* EventBus::peek() const { return getAt(0); }
Event* EventBus::getAt(size_t index) { return index<count_?&events_[index]:nullptr; }
const Event* EventBus::getAt(size_t index) const { return index<count_?&events_[index]:nullptr; }

EventResultCode EventBus::transition(EventId id, EventStatus newStatus, uint32_t timestampMs,
                                     EventResultCode resultCode)
{
    if (!isValidEventStatus(newStatus)) return EventResultCode::INVALID_STATUS;
    Event* event=findById(id);
    if(event==nullptr) return EventResultCode::NOT_FOUND;
    return event->transitionTo(newStatus,timestampMs,resultCode)
        ? EventResultCode::SUCCESS : EventResultCode::INVALID_TRANSITION;
}

void EventBus::removeAt(size_t index)
{
    for(size_t i=index+1U;i<count_;++i) events_[i-1U]=events_[i];
    --count_; events_[count_]=Event{};
}
EventResultCode EventBus::consume(EventId id)
{
    size_t i=findIndexById(id);
    if(i==INVALID_EVENT_INDEX) return EventResultCode::NOT_FOUND;
    if(!events_[i].isTerminal()) return EventResultCode::NOT_TERMINAL;
    removeAt(i); return EventResultCode::SUCCESS;
}
EventResultCode EventBus::consumeFront() { return isEmpty()?EventResultCode::NOT_FOUND:consume(events_[0].id); }
bool EventBus::contains(EventId id) const { return findIndexById(id)!=INVALID_EVENT_INDEX; }
size_t EventBus::size() const { return count_; }
size_t EventBus::capacity() const { return EVENT_BUS_CAPACITY; }
bool EventBus::isFull() const { return count_>=EVENT_BUS_CAPACITY; }
bool EventBus::isEmpty() const { return count_==0; }
