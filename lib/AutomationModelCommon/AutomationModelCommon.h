#ifndef AUTOMATION_MODEL_COMMON_H
#define AUTOMATION_MODEL_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <AutomationCommon.h>

constexpr size_t SCENE_MANAGER_CAPACITY = 16;
constexpr size_t RULE_MANAGER_CAPACITY = 16;
constexpr size_t SCHEDULE_MANAGER_CAPACITY = 16;
constexpr size_t AUTOMATION_NAME_MAX_LENGTH = 40;

enum class AutomationModelResult : uint8_t
{
    SUCCESS = 0, INVALID_MODEL, INVALID_ID, INVALID_NAME, INVALID_STEP,
    INVALID_CONDITION, INVALID_COMMAND, DUPLICATE_ID, DUPLICATE_NAME,
    DUPLICATE_STEP_INDEX, DUPLICATE_COMMAND_INDEX, NOT_FOUND, CAPACITY_FULL,
    STEP_CAPACITY_FULL, COMMAND_CAPACITY_FULL, INVALID_INDEX, BRANCH_MISMATCH,
    SCHEDULE_ID_MISMATCH, OUTPUT_BUFFER_INVALID, OUTPUT_BUFFER_TOO_SMALL, NO_MATCHES
};

inline bool isValidAutomationModelResult(AutomationModelResult result)
{
    switch (result)
    {
        case AutomationModelResult::SUCCESS: case AutomationModelResult::INVALID_MODEL:
        case AutomationModelResult::INVALID_ID: case AutomationModelResult::INVALID_NAME:
        case AutomationModelResult::INVALID_STEP: case AutomationModelResult::INVALID_CONDITION:
        case AutomationModelResult::INVALID_COMMAND: case AutomationModelResult::DUPLICATE_ID:
        case AutomationModelResult::DUPLICATE_NAME: case AutomationModelResult::DUPLICATE_STEP_INDEX:
        case AutomationModelResult::DUPLICATE_COMMAND_INDEX: case AutomationModelResult::NOT_FOUND:
        case AutomationModelResult::CAPACITY_FULL: case AutomationModelResult::STEP_CAPACITY_FULL:
        case AutomationModelResult::COMMAND_CAPACITY_FULL: case AutomationModelResult::INVALID_INDEX:
        case AutomationModelResult::BRANCH_MISMATCH: case AutomationModelResult::SCHEDULE_ID_MISMATCH:
        case AutomationModelResult::OUTPUT_BUFFER_INVALID:
        case AutomationModelResult::OUTPUT_BUFFER_TOO_SMALL: case AutomationModelResult::NO_MATCHES:
            return true;
        default: return false;
    }
}

inline bool isAutomationModelSuccess(AutomationModelResult result)
{ return result == AutomationModelResult::SUCCESS; }

namespace AutomationText
{
inline bool isCanonical(const char* value, size_t capacity)
{
    if (value == nullptr || capacity == 0U) return false;
    size_t end = 0U;
    while (end < capacity && value[end] != '\0') ++end;
    if (end == capacity) return false;
    for (size_t index = end + 1U; index < capacity; ++index)
        if (value[index] != '\0') return false;
    return true;
}

inline bool set(char* destination, size_t capacity, const char* value)
{
    if (destination == nullptr || capacity == 0U || value == nullptr) return false;
    size_t length = 0U;
    while (length < capacity && value[length] != '\0') ++length;
    if (length == capacity) return false;
    for (size_t index = 0U; index < capacity; ++index) destination[index] = '\0';
    for (size_t index = 0U; index < length; ++index) destination[index] = value[index];
    return true;
}
}

#endif
