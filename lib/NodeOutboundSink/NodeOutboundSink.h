#ifndef NODE_OUTBOUND_SINK_H
#define NODE_OUTBOUND_SINK_H
#include <NodeDescriptor.h>
class NodeOutboundSink { public: virtual ~NodeOutboundSink()=default; virtual NodeOutboundSinkResult sendToNode(NodeMessageId,const NodeDescriptor&,const uint8_t*,size_t)=0; virtual NodeOutboundSinkResult updateNodeSend(NodeId)=0; virtual NodeOutboundSinkResult cancelNodeSend(NodeId)=0; };
#endif
