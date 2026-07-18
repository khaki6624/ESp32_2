#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H
#include <EventSink.h>
class EventQueueTestAccess;
class EventQueue final : public EventSink
{
public:
    EventQueue();
    EventPublishResult publish(const Event& event) override;
    const Event* peek() const;
    EventQueueResult consume();
    void clear();
    size_t size() const;
    size_t capacity() const;
    bool empty() const;
    bool full() const;
private:
    // این دسترسی فقط برای ساخت وضعیت فساد کنترل‌شده در تست است.
    friend class EventQueueTestAccess;
    Event events_[EVENT_QUEUE_CAPACITY];
    size_t head_;
    size_t tail_;
    size_t count_;
    bool contains(EventId id) const;
};
#endif
