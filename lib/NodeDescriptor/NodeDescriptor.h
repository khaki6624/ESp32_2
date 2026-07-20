#ifndef NODE_DESCRIPTOR_H
#define NODE_DESCRIPTOR_H
#include <NodeAddress.h>
class NodeDescriptor { public: NodeDescriptor(); static NodeDescriptor create(NodeId,NodeType,const NodeAddress&,NodeDuration,bool); bool isValid()const; NodeId nodeId()const; NodeType nodeType()const; const NodeAddress& address()const; NodeDuration offlineTimeoutMs()const; bool enabled()const; private: NodeId nodeId_; NodeType nodeType_; NodeAddress address_; NodeDuration offlineTimeoutMs_; bool enabled_; bool valid_; };
#endif
