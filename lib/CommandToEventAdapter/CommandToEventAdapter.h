#ifndef COMMAND_TO_EVENT_ADAPTER_H
#define COMMAND_TO_EVENT_ADAPTER_H
#include <CommandEventRegistry.h>
#include <CommandResult.h>
#include <EventSink.h>
class CommandToEventAdapterTestAccess;
class CommandToEventAdapter
{public:CommandToEventAdapter(CommandEventRegistry&,EventSink&,RuntimeIntegrationEventIdProvider&);RuntimeIntegrationResult begin();bool isInitialized()const;RuntimeIntegrationResult onCommandResult(const CommandResult&,uint32_t);uint32_t forwardedCount()const;uint32_t ignoredCount()const;uint32_t failedCount()const;
private:friend class CommandToEventAdapterTestAccess;CommandEventRegistry& registry_;EventSink& sink_;RuntimeIntegrationEventIdProvider& ids_;uint32_t forwarded_,ignored_,failed_;bool initialized_;RuntimeIntegrationResult finish(RuntimeIntegrationResult);};
#endif
