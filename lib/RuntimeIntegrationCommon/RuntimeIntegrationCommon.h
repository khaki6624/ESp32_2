#ifndef RUNTIME_INTEGRATION_COMMON_H
#define RUNTIME_INTEGRATION_COMMON_H
#include <Event.h>

constexpr size_t EVENT_NOTIFICATION_MAX_MAPPINGS=32U;
constexpr size_t NODE_EVENT_MAX_MAPPINGS=16U;
constexpr size_t COMMAND_EVENT_MAX_MAPPINGS=16U;

enum class RuntimeIntegrationResult:uint8_t
{SUCCESS=0,ACCEPTED,IN_PROGRESS,RETRY_LATER,NO_CHANGE,IGNORED,FILTERED,REJECTED,FAILED,INTERNAL_ERROR,NOT_INITIALIZED,INVALID_ARGUMENT,INVALID_INPUT,MAPPING_NOT_FOUND,MAPPING_DISABLED,REGISTRY_FULL,REGISTRY_LOCKED,DUPLICATE_MAPPING,COUNT};
inline bool isValidRuntimeIntegrationResult(RuntimeIntegrationResult v){return static_cast<uint8_t>(v)<static_cast<uint8_t>(RuntimeIntegrationResult::COUNT);}
enum class EventNotificationFormatResult:uint8_t{SUCCESS=0,NO_CONTENT,BUFFER_TOO_SMALL,INVALID_EVENT,FAILED,COUNT};
inline bool isValidEventNotificationFormatResult(EventNotificationFormatResult v){return static_cast<uint8_t>(v)<static_cast<uint8_t>(EventNotificationFormatResult::COUNT);}
enum class NodeEventSignal:uint8_t{ONLINE=0,OFFLINE,DISABLED,SEND_SUCCESS,SEND_FAILURE,SINK_REJECTED,INTERNAL_ERROR,COUNT};
inline bool isValidNodeEventSignal(NodeEventSignal v){return static_cast<uint8_t>(v)<static_cast<uint8_t>(NodeEventSignal::COUNT);}

class RuntimeIntegrationEventIdProvider{public:virtual ~RuntimeIntegrationEventIdProvider()=default;virtual EventId nextEventId()=0;};
class SequentialRuntimeIntegrationEventIdProvider final:public RuntimeIntegrationEventIdProvider
{public:SequentialRuntimeIntegrationEventIdProvider():next_(1U){}EventId nextEventId()override{const EventId id=next_;++next_;if(next_==0U)next_=1U;return id;}private:EventId next_;};
inline void saturatingIncrement(uint32_t& value){if(value!=UINT32_MAX)++value;}
#endif
