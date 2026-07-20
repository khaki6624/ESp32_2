#ifndef COMMAND_EVENT_REGISTRY_H
#define COMMAND_EVENT_REGISTRY_H
#include <CommandEventMapping.h>
class CommandEventRegistry{public:CommandEventRegistry();RuntimeIntegrationResult registerMapping(const CommandEventMapping&);const CommandEventMapping* find(ExecutionStatus,CommandErrorCode)const;const CommandEventMapping* at(size_t)const;size_t size()const;size_t capacity()const;bool isLocked()const;RuntimeIntegrationResult lock();private:CommandEventMapping mappings_[COMMAND_EVENT_MAX_MAPPINGS];size_t size_;bool locked_;};
#endif
