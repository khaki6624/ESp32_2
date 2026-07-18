#ifndef COMMAND_EXECUTION_COMMIT_HOOK_H
#define COMMAND_EXECUTION_COMMIT_HOOK_H

#include <stdint.h>

#include <Command.h>
#include <CommandExecutionCommon.h>

class CommandExecutionCommitHook
{
public:
    virtual ~CommandExecutionCommitHook() = default;

    virtual CommandExecutionGateResult commit(
        const Command& command,
        uint32_t nowMs
    ) = 0;
};

#endif
