#include "CommandExecutor.h"

namespace
{
    CommandErrorCode mapValidationError(CommandValidationResult result)
    {
        switch (result)
        {
            case CommandValidationResult::UNSUPPORTED_COMMAND:
            case CommandValidationResult::DOMAIN_OPERATION_MISMATCH:
                return CommandErrorCode::COMMAND_NOT_SUPPORTED;
            case CommandValidationResult::INVALID_DURATION:
            case CommandValidationResult::DURATION_REQUIRED:
            case CommandValidationResult::DURATION_NOT_ALLOWED:
                return CommandErrorCode::INVALID_DURATION;
            case CommandValidationResult::INVALID_DOMAIN:
                return CommandErrorCode::INVALID_DOMAIN;
            case CommandValidationResult::INVALID_OPERATION:
                return CommandErrorCode::INVALID_OPERATION;
            case CommandValidationResult::INVALID_QUERY:
                return CommandErrorCode::INVALID_QUERY;
            default:
                return CommandErrorCode::INVALID_COMMAND;
        }
    }

    CommandErrorCode mapGateError(CommandExecutionGateResult result)
    {
        switch (result)
        {
            case CommandExecutionGateResult::SYSTEM_MODE_REJECTED:
                return CommandErrorCode::SYSTEM_LOCKED;
            case CommandExecutionGateResult::UNAUTHORIZED:
                return CommandErrorCode::UNAUTHORIZED;
            case CommandExecutionGateResult::CONFIRMATION_REQUIRED:
                return CommandErrorCode::CONFIRMATION_REQUIRED;
            case CommandExecutionGateResult::INVALID_CONFIRM_TOKEN:
                return CommandErrorCode::INVALID_CONFIRM_TOKEN;
            case CommandExecutionGateResult::SAFETY_REJECTED:
                return CommandErrorCode::SAFETY_INTERLOCK;
            case CommandExecutionGateResult::INVALID_COMMAND:
                return CommandErrorCode::INVALID_COMMAND;
            case CommandExecutionGateResult::POLICY_ERROR:
            default:
                return CommandErrorCode::PERMISSION_DENIED;
        }
    }

    CommandErrorCode mapDispatchError(CommandDispatchResult result)
    {
        switch (result)
        {
            case CommandDispatchResult::INVALID_COMMAND:
                return CommandErrorCode::INVALID_COMMAND;
            case CommandDispatchResult::INVALID_DOMAIN:
                return CommandErrorCode::INVALID_DOMAIN;
            case CommandDispatchResult::HANDLER_NOT_FOUND:
            case CommandDispatchResult::UNSUPPORTED_DOMAIN:
                return CommandErrorCode::COMMAND_NOT_SUPPORTED;
            default:
                return CommandErrorCode::HARDWARE_FAILURE;
        }
    }
}

CommandExecutor::CommandExecutor(
    SceneExecutionQueue& commandQueue,
    const CommandValidator& validator,
    const CommandExecutionGate& executionGate,
    const CommandDispatcher& dispatcher
) :
    commandQueue_(commandQueue),
    validator_(validator),
    executionGate_(executionGate),
    dispatcher_(dispatcher),
    state_(CommandExecutorState::IDLE),
    lastResult_(CommandExecutorResult::COMMAND_QUEUE_EMPTY),
    currentCommand_{},
    hasCurrentCommand_(false),
    lastCommandResult_{},
    hasLastCommandResult_(false)
{
}

void CommandExecutor::update(uint32_t nowMs)
{
    switch (state_)
    {
        case CommandExecutorState::IDLE:
        {
            const Command* queued = commandQueue_.peek();
            if (queued == nullptr)
            {
                lastResult_ = CommandExecutorResult::COMMAND_QUEUE_EMPTY;
                return;
            }
            currentCommand_ = *queued;
            hasCurrentCommand_ = true;
            if (commandQueue_.consume() != SceneExecutionResult::SUCCESS)
            {
                clearCurrent();
                finish(CommandExecutorState::FAILED, CommandExecutorResult::INVALID_COMMAND);
                return;
            }
            state_ = CommandExecutorState::LOADING_COMMAND;
            return;
        }
        case CommandExecutorState::LOADING_COMMAND:
            if (!hasCurrentCommand_ || !currentCommand_.isValid())
            {
                storeFailureResult(
                    nowMs, ExecutionStatus::REJECTED, CommandErrorCode::INVALID_COMMAND
                );
                finish(CommandExecutorState::FAILED, CommandExecutorResult::INVALID_COMMAND);
                return;
            }
            state_ = CommandExecutorState::VALIDATING;
            return;
        case CommandExecutorState::VALIDATING:
        {
            const CommandValidationResult validation = validator_.validate(currentCommand_);
            if (validation != CommandValidationResult::VALID)
            {
                const bool stored = storeFailureResult(
                    nowMs, ExecutionStatus::REJECTED, mapValidationError(validation)
                );
                finish(
                    CommandExecutorState::FAILED,
                    stored ? CommandExecutorResult::VALIDATION_FAILED
                           : CommandExecutorResult::COMMAND_RESULT_INVALID
                );
                return;
            }
            state_ = CommandExecutorState::CHECKING_GATE;
            return;
        }
        case CommandExecutorState::CHECKING_GATE:
        {
            const CommandExecutionGateResult gateResult = executionGate_.check(
                currentCommand_, nowMs
            );
            if (gateResult != CommandExecutionGateResult::ALLOWED)
            {
                const bool stored = storeFailureResult(
                    nowMs, ExecutionStatus::REJECTED, mapGateError(gateResult)
                );
                finish(
                    CommandExecutorState::FAILED,
                    stored ? CommandExecutorResult::GATE_REJECTED
                           : CommandExecutorResult::COMMAND_RESULT_INVALID
                );
                return;
            }
            state_ = CommandExecutorState::DISPATCHING;
            return;
        }
        case CommandExecutorState::DISPATCHING:
        {
            CommandResult temporary;
            const CommandDispatchResult dispatchResult = dispatcher_.dispatch(
                currentCommand_, temporary
            );
            if (dispatchResult != CommandDispatchResult::SUCCESS)
            {
                if (dispatchResult == CommandDispatchResult::RESULT_INVALID)
                {
                    storeFailureResult(
                        nowMs,
                        ExecutionStatus::FAILED,
                        CommandErrorCode::HARDWARE_FAILURE
                    );
                    finish(
                        CommandExecutorState::FAILED,
                        CommandExecutorResult::COMMAND_RESULT_INVALID
                    );
                    return;
                }
                const bool stored = storeFailureResult(
                    nowMs, ExecutionStatus::FAILED, mapDispatchError(dispatchResult)
                );
                finish(
                    CommandExecutorState::FAILED,
                    stored ? CommandExecutorResult::DISPATCH_FAILED
                           : CommandExecutorResult::COMMAND_RESULT_INVALID
                );
                return;
            }
            if (!temporary.isValid() || !temporary.isTerminal() ||
                !temporary.isSuccess() ||
                temporary.commandId != currentCommand_.context.commandId ||
                temporary.requestId != currentCommand_.context.request.requestId)
            {
                storeFailureResult(
                    nowMs, ExecutionStatus::FAILED, CommandErrorCode::HARDWARE_FAILURE
                );
                finish(
                    CommandExecutorState::FAILED,
                    CommandExecutorResult::COMMAND_RESULT_INVALID
                );
                return;
            }
            lastCommandResult_ = temporary;
            hasLastCommandResult_ = true;
            finish(CommandExecutorState::COMPLETED, CommandExecutorResult::SUCCESS);
            return;
        }
        case CommandExecutorState::COMPLETED:
        case CommandExecutorState::FAILED:
            clearCurrent();
            state_ = CommandExecutorState::IDLE;
            return;
        default:
            clearCurrent();
            finish(CommandExecutorState::FAILED, CommandExecutorResult::INVALID_COMMAND);
            return;
    }
}

CommandExecutorState CommandExecutor::getState() const { return state_; }
CommandExecutorResult CommandExecutor::getLastResult() const { return lastResult_; }
const CommandResult* CommandExecutor::getLastCommandResult() const
{
    return hasLastCommandResult_ ? &lastCommandResult_ : nullptr;
}
bool CommandExecutor::hasLastCommandResult() const { return hasLastCommandResult_; }

void CommandExecutor::clearLastCommandResult()
{
    lastCommandResult_ = CommandResult{};
    hasLastCommandResult_ = false;
}

void CommandExecutor::reset()
{
    clearCurrent();
    clearLastCommandResult();
    state_ = CommandExecutorState::IDLE;
    lastResult_ = CommandExecutorResult::COMMAND_QUEUE_EMPTY;
}

void CommandExecutor::finish(
    CommandExecutorState terminalState,
    CommandExecutorResult result
)
{
    state_ = terminalState;
    lastResult_ = result;
}

void CommandExecutor::clearCurrent()
{
    currentCommand_ = Command{};
    hasCurrentCommand_ = false;
}

bool CommandExecutor::storeFailureResult(
    uint32_t nowMs,
    ExecutionStatus status,
    CommandErrorCode errorCode
)
{
    if (!hasCurrentCommand_ || currentCommand_.context.commandId == INVALID_COMMAND_ID ||
        currentCommand_.context.request.requestId == INVALID_REQUEST_ID)
        return false;

    CommandResult temporary;
    temporary.commandId = currentCommand_.context.commandId;
    temporary.requestId = currentCommand_.context.request.requestId;
    if (!temporary.isValid())
        return false;
    if (status == ExecutionStatus::REJECTED)
    {
        if (!temporary.transitionTo(status, nowMs, errorCode))
            return false;
    }
    else if (status == ExecutionStatus::FAILED)
    {
        if (!temporary.transitionTo(ExecutionStatus::VALIDATING, nowMs) ||
            !temporary.transitionTo(ExecutionStatus::ACCEPTED, nowMs) ||
            !temporary.transitionTo(ExecutionStatus::EXECUTING, nowMs) ||
            !temporary.transitionTo(ExecutionStatus::FAILED, nowMs, errorCode))
            return false;
    }
    else
    {
        return false;
    }
    if (!temporary.isValid() || !temporary.isTerminal())
        return false;
    lastCommandResult_ = temporary;
    hasLastCommandResult_ = true;
    return true;
}
