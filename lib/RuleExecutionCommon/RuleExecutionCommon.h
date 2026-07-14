#ifndef RULE_EXECUTION_COMMON_H
#define RULE_EXECUTION_COMMON_H

#include <stddef.h>
#include <stdint.h>

enum class RuleExecutionState : uint8_t
{
    IDLE = 0, LOADING_TRIGGER, RESERVING_COMMAND_IDS, READY_ACTION,
    WAITING_DELAY, WAITING_COMMAND_SINK, COMPLETED, FAILED, CANCELLED
};

inline bool isValidRuleExecutionState(RuleExecutionState state)
{
    switch (state)
    {
        case RuleExecutionState::IDLE: case RuleExecutionState::LOADING_TRIGGER:
        case RuleExecutionState::RESERVING_COMMAND_IDS: case RuleExecutionState::READY_ACTION:
        case RuleExecutionState::WAITING_DELAY: case RuleExecutionState::WAITING_COMMAND_SINK:
        case RuleExecutionState::COMPLETED: case RuleExecutionState::FAILED:
        case RuleExecutionState::CANCELLED: return true;
        default: return false;
    }
}

inline bool isTerminalRuleExecutionState(RuleExecutionState state)
{
    return state == RuleExecutionState::COMPLETED || state == RuleExecutionState::FAILED ||
           state == RuleExecutionState::CANCELLED;
}

enum class RuleExecutionResult : uint8_t
{
    SUCCESS = 0, INVALID_TRIGGER, INVALID_RULE_ID, INVALID_BRANCH, RULE_NOT_FOUND,
    RULE_DISABLED, RULE_INVALID, ACTION_NOT_FOUND, INVALID_ACTION,
    COMMAND_ID_RESERVATION_FAILED, INVALID_COMMAND_ID_RANGE, COMMAND_FACTORY_FAILED,
    COMMAND_SINK_FULL, COMMAND_SINK_REJECTED, NOT_RUNNING, CANCELLED, QUEUE_EMPTY,
    RULE_CHANGED_DURING_EXECUTION
};

inline bool isValidRuleExecutionResult(RuleExecutionResult result)
{
    switch (result)
    {
        case RuleExecutionResult::SUCCESS: case RuleExecutionResult::INVALID_TRIGGER:
        case RuleExecutionResult::INVALID_RULE_ID: case RuleExecutionResult::INVALID_BRANCH:
        case RuleExecutionResult::RULE_NOT_FOUND: case RuleExecutionResult::RULE_DISABLED:
        case RuleExecutionResult::RULE_INVALID: case RuleExecutionResult::ACTION_NOT_FOUND:
        case RuleExecutionResult::INVALID_ACTION:
        case RuleExecutionResult::COMMAND_ID_RESERVATION_FAILED:
        case RuleExecutionResult::INVALID_COMMAND_ID_RANGE:
        case RuleExecutionResult::COMMAND_FACTORY_FAILED:
        case RuleExecutionResult::COMMAND_SINK_FULL:
        case RuleExecutionResult::COMMAND_SINK_REJECTED:
        case RuleExecutionResult::NOT_RUNNING: case RuleExecutionResult::CANCELLED:
        case RuleExecutionResult::QUEUE_EMPTY:
        case RuleExecutionResult::RULE_CHANGED_DURING_EXECUTION: return true;
        default: return false;
    }
}

enum class CommandIdReservationResult : uint8_t
{ SUCCESS = 0, INVALID_COUNT, RANGE_UNAVAILABLE, PROVIDER_ERROR };

inline bool isValidCommandIdReservationResult(CommandIdReservationResult result)
{
    switch (result)
    {
        case CommandIdReservationResult::SUCCESS: case CommandIdReservationResult::INVALID_COUNT:
        case CommandIdReservationResult::RANGE_UNAVAILABLE:
        case CommandIdReservationResult::PROVIDER_ERROR: return true;
        default: return false;
    }
}

enum class RuntimeCommandSubmitResult : uint8_t
{ SUCCESS = 0, INVALID_COMMAND, SINK_FULL, SINK_ERROR };

inline bool isValidRuntimeCommandSubmitResult(RuntimeCommandSubmitResult result)
{
    switch (result)
    {
        case RuntimeCommandSubmitResult::SUCCESS: case RuntimeCommandSubmitResult::INVALID_COMMAND:
        case RuntimeCommandSubmitResult::SINK_FULL: case RuntimeCommandSubmitResult::SINK_ERROR:
            return true;
        default: return false;
    }
}

#endif
