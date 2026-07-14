#ifndef SCHEDULER_COMMON_H
#define SCHEDULER_COMMON_H

#include <AutomationModelCommon.h>

constexpr size_t SCHEDULER_RUNTIME_STATE_CAPACITY =
    SCHEDULE_MANAGER_CAPACITY * SCHEDULE_MAX_COMMANDS;

enum class SchedulerState : uint8_t
{ IDLE = 0, CHECKING, WAITING_COMMAND_ID, WAITING_COMMAND_SINK, COMPLETED, FAILED };

inline bool isValidSchedulerState(SchedulerState value)
{
    switch (value)
    {
        case SchedulerState::IDLE: case SchedulerState::CHECKING:
        case SchedulerState::WAITING_COMMAND_ID: case SchedulerState::WAITING_COMMAND_SINK:
        case SchedulerState::COMPLETED: case SchedulerState::FAILED: return true;
        default: return false;
    }
}
inline bool isTerminalSchedulerState(SchedulerState value)
{ return value == SchedulerState::COMPLETED || value == SchedulerState::FAILED; }

enum class SchedulerResult : uint8_t
{
    SUCCESS = 0, INVALID_TIME_SNAPSHOT, TIME_UNAVAILABLE, SCHEDULE_NOT_FOUND,
    SCHEDULE_DISABLED, SCHEDULE_INVALID, COMMAND_NOT_FOUND, COMMAND_DISABLED,
    COMMAND_INVALID, SOLAR_TIME_UNAVAILABLE, INVALID_SOLAR_TIME,
    COMMAND_ID_RESERVATION_FAILED, INVALID_COMMAND_ID, COMMAND_FACTORY_FAILED,
    COMMAND_SINK_FULL, COMMAND_SINK_REJECTED, SCHEDULE_CHANGED_DURING_EXECUTION,
    RUNTIME_STATE_FULL, NOT_DUE, QUEUE_EMPTY, INVALID_REQUEST_CONTEXT
};
inline bool isValidSchedulerResult(SchedulerResult value)
{
    switch (value)
    {
        case SchedulerResult::SUCCESS: case SchedulerResult::INVALID_TIME_SNAPSHOT:
        case SchedulerResult::TIME_UNAVAILABLE: case SchedulerResult::SCHEDULE_NOT_FOUND:
        case SchedulerResult::SCHEDULE_DISABLED: case SchedulerResult::SCHEDULE_INVALID:
        case SchedulerResult::COMMAND_NOT_FOUND: case SchedulerResult::COMMAND_DISABLED:
        case SchedulerResult::COMMAND_INVALID: case SchedulerResult::SOLAR_TIME_UNAVAILABLE:
        case SchedulerResult::INVALID_SOLAR_TIME:
        case SchedulerResult::COMMAND_ID_RESERVATION_FAILED:
        case SchedulerResult::INVALID_COMMAND_ID: case SchedulerResult::COMMAND_FACTORY_FAILED:
        case SchedulerResult::COMMAND_SINK_FULL: case SchedulerResult::COMMAND_SINK_REJECTED:
        case SchedulerResult::SCHEDULE_CHANGED_DURING_EXECUTION:
        case SchedulerResult::RUNTIME_STATE_FULL: case SchedulerResult::NOT_DUE:
        case SchedulerResult::QUEUE_EMPTY: case SchedulerResult::INVALID_REQUEST_CONTEXT:
            return true;
        default: return false;
    }
}

enum class TimeProviderResult : uint8_t { SUCCESS = 0, TIME_UNAVAILABLE, INVALID_TIME };
inline bool isValidTimeProviderResult(TimeProviderResult value)
{
    switch (value)
    { case TimeProviderResult::SUCCESS: case TimeProviderResult::TIME_UNAVAILABLE:
      case TimeProviderResult::INVALID_TIME: return true; default: return false; }
}

enum class SolarTimeProviderResult : uint8_t
{ SUCCESS = 0, DATE_UNAVAILABLE, SUNRISE_UNAVAILABLE, SUNSET_UNAVAILABLE, INVALID_RESULT };
inline bool isValidSolarTimeProviderResult(SolarTimeProviderResult value)
{
    switch (value)
    { case SolarTimeProviderResult::SUCCESS: case SolarTimeProviderResult::DATE_UNAVAILABLE:
      case SolarTimeProviderResult::SUNRISE_UNAVAILABLE:
      case SolarTimeProviderResult::SUNSET_UNAVAILABLE:
      case SolarTimeProviderResult::INVALID_RESULT: return true; default: return false; }
}

#endif
