#ifndef NODE_RUNTIME_REGISTRY_H
#define NODE_RUNTIME_REGISTRY_H
#include <NodeDescriptor.h>
class NodeRuntimeRegistry { public: NodeRuntimeRegistry(); NodeRuntimeResult registerNode(const NodeDescriptor&); const NodeDescriptor* findById(NodeId)const; const NodeDescriptor* findByAddress(const NodeAddress&)const; const NodeDescriptor* at(size_t)const; size_t size()const; size_t capacity()const; bool isLocked()const; NodeRuntimeResult lock(); private: NodeDescriptor nodes_[NODE_RUNTIME_MAX_NODES]; size_t size_; bool locked_; };
#endif
