#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Command.h>
#include <CommandExecutionCommon.h>
#include <CommandResult.h>

class CommandHandler
{
public:
    virtual ~CommandHandler() = default;
    virtual bool supports(const Command& command) const = 0;
    virtual CommandDispatchResult handle(
        const Command& command,
        CommandResult& output
    ) const = 0;
};

#endif
