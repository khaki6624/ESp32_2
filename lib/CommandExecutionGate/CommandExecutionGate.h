#ifndef COMMAND_EXECUTION_GATE_H
#define COMMAND_EXECUTION_GATE_H

#include <Command.h>
#include <CommandExecutionCommon.h>

class CommandExecutionGate
{
public:
    virtual ~CommandExecutionGate() = default;
    virtual CommandExecutionGateResult check(
        const Command& command,
        uint32_t nowMs
    ) const = 0;
};

#endif
