#ifndef EVENT_SINK_H
#define EVENT_SINK_H
#include <Event.h>
class EventSink
{
public:
    virtual ~EventSink() = default;
    virtual EventPublishResult publish(const Event& event) = 0;
};
#endif
