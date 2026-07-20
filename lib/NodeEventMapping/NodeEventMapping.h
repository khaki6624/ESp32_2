#ifndef NODE_EVENT_MAPPING_H
#define NODE_EVENT_MAPPING_H
#include <RuntimeIntegrationCommon.h>
class NodeEventMapping{public:NodeEventMapping();static NodeEventMapping create(NodeEventSignal,EventType,EventSeverity,bool);bool isValid()const;NodeEventSignal signal()const;EventType eventType()const;EventSeverity severity()const;bool enabled()const;private:NodeEventSignal signal_;EventType eventType_;EventSeverity severity_;bool enabled_;bool valid_;};
#endif
