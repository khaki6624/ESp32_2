#ifndef NODE_ADDRESS_RUNTIME_H
#define NODE_ADDRESS_RUNTIME_H
#include <NodeRuntimeCommon.h>
class NodeAddress { public: NodeAddress(); bool set(const char* value); bool isValid() const; const char* c_str() const; size_t length() const; bool equals(const NodeAddress& other) const; private: char value_[NODE_RUNTIME_MAX_ADDRESS_LENGTH]; };
#endif
