#ifndef NODE_MESSAGE_SINK_H
#define NODE_MESSAGE_SINK_H
#include <NodeInboundMessage.h>
class NodeMessageSink { public: virtual ~NodeMessageSink()=default; virtual NodeMessageSinkResult onNodeMessage(const NodeInboundMessage&)=0; };
#endif
