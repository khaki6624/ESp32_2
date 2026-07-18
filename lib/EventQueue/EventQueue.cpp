#include "EventQueue.h"
EventQueue::EventQueue() : events_{}, head_(0U), tail_(0U), count_(0U) {}
EventPublishResult EventQueue::publish(const Event& event)
{
    if (!event.isValid()) return EventPublishResult::INVALID_EVENT;
    if (full()) return EventPublishResult::QUEUE_FULL;
    if (contains(event.id)) return EventPublishResult::DUPLICATE_EVENT;
    events_[tail_] = event;
    tail_ = (tail_ + 1U) % EVENT_QUEUE_CAPACITY;
    ++count_;
    return EventPublishResult::SUCCESS;
}
const Event* EventQueue::peek() const { return empty() ? nullptr : &events_[head_]; }
EventQueueResult EventQueue::consume()
{
    if (empty()) return EventQueueResult::QUEUE_EMPTY;
    events_[head_] = Event{};
    head_ = (head_ + 1U) % EVENT_QUEUE_CAPACITY;
    --count_;
    return EventQueueResult::SUCCESS;
}
void EventQueue::clear()
{
    for (size_t index = 0U; index < EVENT_QUEUE_CAPACITY; ++index) events_[index] = Event{};
    head_ = 0U; tail_ = 0U; count_ = 0U;
}
size_t EventQueue::size() const { return count_; }
size_t EventQueue::capacity() const { return EVENT_QUEUE_CAPACITY; }
bool EventQueue::empty() const { return count_ == 0U; }
bool EventQueue::full() const { return count_ == EVENT_QUEUE_CAPACITY; }
bool EventQueue::contains(EventId id) const
{
    for (size_t offset = 0U; offset < count_; ++offset)
        if (events_[(head_ + offset) % EVENT_QUEUE_CAPACITY].id == id) return true;
    return false;
}
