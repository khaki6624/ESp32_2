#ifndef RUNTIME_COMMAND_SINK_H
#define RUNTIME_COMMAND_SINK_H

#include <Command.h>
#include <RuleExecutionCommon.h>

class RuntimeCommandSink
{
public:
    virtual ~RuntimeCommandSink() = default;
    virtual RuntimeCommandSubmitResult submit(const Command& command) = 0;
    virtual bool isFull() const = 0;
};

#endif
