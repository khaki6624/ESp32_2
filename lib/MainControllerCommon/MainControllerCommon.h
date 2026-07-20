#ifndef MAIN_CONTROLLER_COMMON_H
#define MAIN_CONTROLLER_COMMON_H
#include <stddef.h>
#include <stdint.h>

using MainControllerTimestamp=uint32_t;
enum class MainControllerResult:uint8_t{SUCCESS=0,IN_PROGRESS,RETRY_LATER,NO_CHANGE,DEGRADED,COMPONENT_FAILED,INTERNAL_ERROR,NOT_INITIALIZED,ALREADY_UPDATING,INVALID_ARGUMENT,INITIALIZATION_FAILED,COUNT};
inline bool isValidMainControllerResult(MainControllerResult v){return static_cast<uint8_t>(v)<static_cast<uint8_t>(MainControllerResult::COUNT);}
enum class MainControllerComponent:uint8_t{CONFIGURATION=0,STORAGE,COMMUNICATION,COMMAND_EXECUTION,NODE,SCHEDULER,RULE,SCENE,EVENT,EVENT_TO_NOTIFICATION,NODE_TO_EVENT,COMMAND_TO_EVENT,NOTIFICATION,LOGGER,COUNT};
inline bool isValidMainControllerComponent(MainControllerComponent v){return static_cast<uint8_t>(v)<static_cast<uint8_t>(MainControllerComponent::COUNT);}
constexpr size_t MAIN_CONTROLLER_COMPONENT_COUNT=static_cast<size_t>(MainControllerComponent::COUNT);
inline void mainControllerSaturatingIncrement(uint32_t& value){if(value!=UINT32_MAX)++value;}
#endif
