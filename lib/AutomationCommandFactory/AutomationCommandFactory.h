#ifndef AUTOMATION_COMMAND_FACTORY_H
#define AUTOMATION_COMMAND_FACTORY_H

#include <AutomationCommand.h>
#include <AutomationExecutionCommon.h>
#include <Command.h>

class AutomationCommandFactory
{
public:
    AutomationCommandFactory();
    AutomationCommandFactoryResult create(const AutomationCommand& source,
        const RequestContext& request,CommandId commandId,uint32_t createdTimestampMs,
        Command& output) const;
private:
    static bool mapRisk(const AutomationCommand& source,CommandRisk& output);
    static bool convertArgument(const AutomationCommandArgument& source,CommandArgument& output);
};

#endif
