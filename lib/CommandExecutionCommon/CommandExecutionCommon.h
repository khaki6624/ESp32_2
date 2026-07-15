#ifndef COMMAND_EXECUTION_COMMON_H
#define COMMAND_EXECUTION_COMMON_H

#include <stdint.h>

enum class CommandDispatchResult : uint8_t
{
    SUCCESS = 0,
    INVALID_COMMAND,
    INVALID_DOMAIN,
    HANDLER_NOT_FOUND,
    UNSUPPORTED_DOMAIN,
    HANDLER_REJECTED,
    RESULT_INVALID,
    HANDLER_ALREADY_REGISTERED,
    DISPATCHER_FULL,
    HANDLER_NOT_REGISTERED
};

inline bool isValidCommandDispatchResult(CommandDispatchResult result)
{
    switch (result)
    {
        case CommandDispatchResult::SUCCESS:
        case CommandDispatchResult::INVALID_COMMAND:
        case CommandDispatchResult::INVALID_DOMAIN:
        case CommandDispatchResult::HANDLER_NOT_FOUND:
        case CommandDispatchResult::UNSUPPORTED_DOMAIN:
        case CommandDispatchResult::HANDLER_REJECTED:
        case CommandDispatchResult::RESULT_INVALID:
        case CommandDispatchResult::HANDLER_ALREADY_REGISTERED:
        case CommandDispatchResult::DISPATCHER_FULL:
        case CommandDispatchResult::HANDLER_NOT_REGISTERED:
            return true;
        default:
            return false;
    }
}

enum class CommandExecutionGateResult : uint8_t
{
    ALLOWED = 0,
    INVALID_COMMAND,
    SYSTEM_MODE_REJECTED,
    UNAUTHORIZED,
    CONFIRMATION_REQUIRED,
    INVALID_CONFIRM_TOKEN,
    SAFETY_REJECTED,
    POLICY_ERROR
};

inline bool isValidCommandExecutionGateResult(CommandExecutionGateResult result)
{
    switch (result)
    {
        case CommandExecutionGateResult::ALLOWED:
        case CommandExecutionGateResult::INVALID_COMMAND:
        case CommandExecutionGateResult::SYSTEM_MODE_REJECTED:
        case CommandExecutionGateResult::UNAUTHORIZED:
        case CommandExecutionGateResult::CONFIRMATION_REQUIRED:
        case CommandExecutionGateResult::INVALID_CONFIRM_TOKEN:
        case CommandExecutionGateResult::SAFETY_REJECTED:
        case CommandExecutionGateResult::POLICY_ERROR:
            return true;
        default:
            return false;
    }
}

enum class CommandExecutorState : uint8_t
{
    IDLE = 0,
    LOADING_COMMAND,
    VALIDATING,
    CHECKING_GATE,
    DISPATCHING,
    COMPLETED,
    FAILED
};

inline bool isValidCommandExecutorState(CommandExecutorState state)
{
    switch (state)
    {
        case CommandExecutorState::IDLE:
        case CommandExecutorState::LOADING_COMMAND:
        case CommandExecutorState::VALIDATING:
        case CommandExecutorState::CHECKING_GATE:
        case CommandExecutorState::DISPATCHING:
        case CommandExecutorState::COMPLETED:
        case CommandExecutorState::FAILED:
            return true;
        default:
            return false;
    }
}

enum class CommandExecutorResult : uint8_t
{
    SUCCESS = 0,
    COMMAND_QUEUE_EMPTY,
    INVALID_COMMAND,
    VALIDATION_FAILED,
    GATE_REJECTED,
    DISPATCH_FAILED,
    COMMAND_RESULT_INVALID
};

inline bool isValidCommandExecutorResult(CommandExecutorResult result)
{
    switch (result)
    {
        case CommandExecutorResult::SUCCESS:
        case CommandExecutorResult::COMMAND_QUEUE_EMPTY:
        case CommandExecutorResult::INVALID_COMMAND:
        case CommandExecutorResult::VALIDATION_FAILED:
        case CommandExecutorResult::GATE_REJECTED:
        case CommandExecutorResult::DISPATCH_FAILED:
        case CommandExecutorResult::COMMAND_RESULT_INVALID:
            return true;
        default:
            return false;
    }
}

#endif
