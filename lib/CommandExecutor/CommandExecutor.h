#ifndef COMMAND_EXECUTOR_H
#define COMMAND_EXECUTOR_H

#include <CommandDispatcher.h>
#include <CommandExecutionCommitHook.h>
#include <CommandExecutionGate.h>
#include <CommandValidator.h>
#include <SceneExecutionQueue.h>

class CommandExecutor
{
public:
    CommandExecutor(
        SceneExecutionQueue& commandQueue,
        const CommandValidator& validator,
        const CommandExecutionGate& executionGate,
        CommandExecutionCommitHook& commitHook,
        const CommandDispatcher& dispatcher
    );

    void update(uint32_t nowMs);
    CommandExecutorState getState() const;
    CommandExecutorResult getLastResult() const;
    const CommandResult* getLastCommandResult() const;
    bool hasLastCommandResult() const;
    void clearLastCommandResult();
    void reset();

private:
    SceneExecutionQueue& commandQueue_;
    const CommandValidator& validator_;
    const CommandExecutionGate& executionGate_;
    CommandExecutionCommitHook& commitHook_;
    const CommandDispatcher& dispatcher_;
    CommandExecutorState state_;
    CommandExecutorResult lastResult_;
    Command currentCommand_;
    bool hasCurrentCommand_;
    CommandResult lastCommandResult_;
    bool hasLastCommandResult_;

    void finish(
        CommandExecutorState terminalState,
        CommandExecutorResult result
    );
    void clearCurrent();
    bool storeFailureResult(
        uint32_t nowMs,
        ExecutionStatus status,
        CommandErrorCode errorCode
    );
};

#endif
