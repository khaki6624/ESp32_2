#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <Event.h>

class EventBus
{
public:
    EventBus();
    void clear();
    EventResultCode publish(const Event& event);
    Event* findById(EventId id);
    const Event* findById(EventId id) const;
    Event* peek();
    const Event* peek() const;
    Event* getAt(size_t index);
    const Event* getAt(size_t index) const;
    EventResultCode transition(EventId id, EventStatus newStatus, uint32_t timestampMs,
                               EventResultCode resultCode = EventResultCode::NONE);
    EventResultCode consume(EventId id);
    EventResultCode consumeFront();
    bool contains(EventId id) const;
    size_t size() const;
    size_t capacity() const;
    bool isFull() const;
    bool isEmpty() const;

private:
    Event events_[EVENT_BUS_CAPACITY];
    size_t count_;
    size_t findIndexById(EventId id) const;
    void removeAt(size_t index);
};

#endif
