#ifndef NODE_RUNTIME_H
#define NODE_RUNTIME_H
#include <NodeRuntimeRegistry.h>
#include <NodeRuntimeState.h>
#include <NodeMessageSink.h>
#include <NodeOutboundSink.h>
struct NodeRuntimeSlot { const NodeDescriptor* descriptor; NodeRuntimeState state; bool hasBeenSeen; NodeRuntimeSlot():descriptor(nullptr),state{},hasBeenSeen(false){} };
class NodeRuntime
{
public:
 NodeRuntime(NodeRuntimeRegistry&,NodeMessageSink&,NodeOutboundSink&);
 NodeRuntimeResult begin(NodeTimestamp); bool isInitialized()const; NodeRuntimeResult update(NodeTimestamp);
 NodeRuntimeResult handleInbound(const NodeInboundMessage&,NodeTimestamp);
 NodeRuntimeResult send(NodeMessageId,NodeId,const uint8_t*,size_t); NodeRuntimeResult cancelSend(NodeId);
 const NodeRuntimeState* state(NodeId)const; bool isOnline(NodeId)const; bool isBusy(NodeId)const;
private:
 NodeRuntimeRegistry& registry_; NodeMessageSink& inboundSink_; NodeOutboundSink& outboundSink_;
 NodeRuntimeSlot slots_[NODE_RUNTIME_MAX_NODES]; size_t slotCount_; bool initialized_;
 size_t findSlot(NodeId)const; static NodeRuntimeResult higherPriority(NodeRuntimeResult,NodeRuntimeResult);
};
#endif
