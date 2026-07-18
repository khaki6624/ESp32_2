#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H
#include <Event.h>
class EventHandler
{
public:
    virtual ~EventHandler() = default;
    virtual EventHandleResult handle(const Event& event) = 0;
};
#endif
