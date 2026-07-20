#ifndef NODE_EVENT_REGISTRY_H
#define NODE_EVENT_REGISTRY_H
#include <NodeEventMapping.h>
class NodeEventRegistry{public:NodeEventRegistry();RuntimeIntegrationResult registerMapping(const NodeEventMapping&);const NodeEventMapping* find(NodeEventSignal)const;const NodeEventMapping* at(size_t)const;size_t size()const;size_t capacity()const;bool isLocked()const;RuntimeIntegrationResult lock();private:NodeEventMapping mappings_[NODE_EVENT_MAX_MAPPINGS];size_t size_;bool locked_;};
#endif
