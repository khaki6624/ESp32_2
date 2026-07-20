#ifndef COMMAND_EVENT_MAPPING_H
#define COMMAND_EVENT_MAPPING_H
#include <CommandCommon.h>
#include <RuntimeIntegrationCommon.h>
class CommandEventMapping{public:CommandEventMapping();static CommandEventMapping create(ExecutionStatus,CommandErrorCode,EventType,EventSeverity,bool);bool isValid()const;ExecutionStatus status()const;CommandErrorCode errorCode()const;EventType eventType()const;EventSeverity severity()const;bool enabled()const;private:ExecutionStatus status_;CommandErrorCode errorCode_;EventType eventType_;EventSeverity severity_;bool enabled_;bool valid_;};
#endif
