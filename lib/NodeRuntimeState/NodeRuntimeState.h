#ifndef NODE_RUNTIME_STATE_H
#define NODE_RUNTIME_STATE_H
#include <NodeRuntimeCommon.h>
class NodeRuntime;
class NodeRuntimeState { public: NodeRuntimeState(); bool isValid()const; NodeId nodeId()const; NodeConnectionState connectionState()const; NodeTimestamp lastSeenAt()const; NodeTimestamp lastStateChangeAt()const; NodeRuntimeResult lastResult()const; bool outboundBusy()const; NodeMessageId activeOutboundMessageId()const; private: friend class NodeRuntime; NodeId nodeId_; NodeConnectionState connectionState_; NodeTimestamp lastSeenAt_; NodeTimestamp lastStateChangeAt_; NodeRuntimeResult lastResult_; NodeMessageId activeOutboundMessageId_; bool outboundBusy_; };
#endif
