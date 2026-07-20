#ifndef NODE_INBOUND_MESSAGE_H
#define NODE_INBOUND_MESSAGE_H
#include <NodeAddress.h>
class NodeInboundMessage { public: NodeInboundMessage(); static NodeInboundMessage create(NodeMessageId,NodeId,const NodeAddress&,const uint8_t*,size_t); bool isValid()const; NodeMessageId messageId()const; NodeId sourceNodeId()const; const NodeAddress& sourceAddress()const; const uint8_t* payload()const; size_t payloadLength()const; private: NodeMessageId messageId_; NodeId sourceNodeId_; NodeAddress sourceAddress_; const uint8_t* payload_; size_t payloadLength_; bool valid_; };
#endif
