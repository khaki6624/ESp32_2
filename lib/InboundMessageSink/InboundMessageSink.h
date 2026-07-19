#ifndef INBOUND_MESSAGE_SINK_H
#define INBOUND_MESSAGE_SINK_H
#include <CommunicationMessage.h>
class InboundMessageSink
{
public:
 virtual ~InboundMessageSink()=default;
 virtual InboundMessageSinkResult onMessage(const CommunicationMessage& message)=0;
};
// Message و Payload فقط در طول onMessage معتبر تضمین می‌شوند و Sink مالک آن‌ها نیست.
#endif
