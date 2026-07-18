#ifndef EVENT_DISPATCHER_H
#define EVENT_DISPATCHER_H
#include <EventHandler.h>
#include <EventQueue.h>
class EventDispatcher
{
public:
    explicit EventDispatcher(EventQueue& queue);
    EventHandlerRegistrationResult addHandler(EventHandler& handler);
    EventHandlerRegistrationResult removeHandler(EventHandler& handler);
    EventDispatchResult update();
    size_t handlerCount() const;
    size_t handlerCapacity() const;
private:
    EventQueue& queue_;
    // مالکیت Handlerها نزد فراخواننده است و طول عمرشان باید از Registration بیشتر باشد.
    EventHandler* handlers_[EVENT_HANDLER_CAPACITY];
    size_t handlerCount_;
    bool dispatching_;
};
#endif
