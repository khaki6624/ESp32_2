#ifndef NODE_TO_EVENT_ADAPTER_H
#define NODE_TO_EVENT_ADAPTER_H
#include <EventSink.h>
#include <NodeEventRegistry.h>
#include <NodeRuntimeState.h>
class NodeToEventAdapterTestAccess;
class NodeToEventAdapter
{public:NodeToEventAdapter(NodeEventRegistry&,EventSink&,RuntimeIntegrationEventIdProvider&);RuntimeIntegrationResult begin();bool isInitialized()const;RuntimeIntegrationResult onNodeStateChanged(const NodeRuntimeState&,const NodeRuntimeState&,NodeTimestamp);RuntimeIntegrationResult onNodeResult(NodeId,NodeRuntimeResult,NodeTimestamp);uint32_t forwardedCount()const;uint32_t ignoredCount()const;uint32_t failedCount()const;
private:friend class NodeToEventAdapterTestAccess;NodeEventRegistry& registry_;EventSink& sink_;RuntimeIntegrationEventIdProvider& ids_;uint32_t forwarded_,ignored_,failed_;bool initialized_;RuntimeIntegrationResult emit(NodeId,NodeEventSignal,NodeTimestamp,NodeRuntimeResult);RuntimeIntegrationResult finish(RuntimeIntegrationResult);static bool signalForState(NodeConnectionState,NodeEventSignal&);static bool signalForResult(NodeRuntimeResult,NodeEventSignal&);};
#endif
