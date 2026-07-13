#ifndef AUTOMATION_COMMON_H
#define AUTOMATION_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <CommandCommon.h>

using SceneId = uint16_t;
using RuleId = uint16_t;
using ScheduleId = uint16_t;
using AutomationStepIndex = uint16_t;

constexpr SceneId INVALID_SCENE_ID = 0;
constexpr RuleId INVALID_RULE_ID = 0;
constexpr ScheduleId INVALID_SCHEDULE_ID = 0;
constexpr AutomationStepIndex INVALID_AUTOMATION_STEP_INDEX = 0;
constexpr size_t SCENE_MAX_STEPS = 16;
constexpr size_t RULE_MAX_ACTION_STEPS = 16;
constexpr size_t SCHEDULE_MAX_COMMANDS = 8;

enum class RuleBranch : uint8_t { NONE = 0, THEN_BRANCH, ELSE_BRANCH };
inline bool isValidRuleBranch(RuleBranch value)
{
    switch(value)
    {
        case RuleBranch::NONE: case RuleBranch::THEN_BRANCH: case RuleBranch::ELSE_BRANCH: return true;
        default: return false;
    }
}

enum class ScheduleMode : uint8_t
{ NONE = 0, FIXED_TIME, FIXED_DATE_TIME, INTERVAL, SUNRISE_OFFSET, SUNSET_OFFSET, MANUAL };
inline bool isValidScheduleMode(ScheduleMode value)
{
    switch(value)
    {
        case ScheduleMode::NONE: case ScheduleMode::FIXED_TIME: case ScheduleMode::FIXED_DATE_TIME:
        case ScheduleMode::INTERVAL: case ScheduleMode::SUNRISE_OFFSET:
        case ScheduleMode::SUNSET_OFFSET: case ScheduleMode::MANUAL: return true;
        default: return false;
    }
}

enum class DayOfWeek : uint8_t
{ NONE = 0, MONDAY = 1, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY, SUNDAY };
inline bool isValidDayOfWeek(DayOfWeek value)
{
    switch(value)
    {
        case DayOfWeek::NONE: case DayOfWeek::MONDAY: case DayOfWeek::TUESDAY:
        case DayOfWeek::WEDNESDAY: case DayOfWeek::THURSDAY: case DayOfWeek::FRIDAY:
        case DayOfWeek::SATURDAY: case DayOfWeek::SUNDAY: return true;
        default: return false;
    }
}

using ScheduleDaysMask = uint8_t;
inline ScheduleDaysMask dayToMask(DayOfWeek day)
{
    if(!isValidDayOfWeek(day) || day==DayOfWeek::NONE) return 0;
    return static_cast<ScheduleDaysMask>(1U << (static_cast<uint8_t>(day)-1U));
}
inline bool hasDay(ScheduleDaysMask mask,DayOfWeek day)
{ const ScheduleDaysMask bit=dayToMask(day);return bit!=0U&&(mask&bit)!=0U; }
inline void addDay(ScheduleDaysMask& mask,DayOfWeek day) { mask|=dayToMask(day); }
inline void removeDay(ScheduleDaysMask& mask,DayOfWeek day) { mask&=static_cast<ScheduleDaysMask>(~dayToMask(day)); }
inline ScheduleDaysMask allDaysMask() { return 0x7FU; }
inline bool isValidScheduleDaysMask(ScheduleDaysMask mask) { return (mask&0x80U)==0U; }

enum class AutomationValidationResult : uint8_t
{
    VALID = 0, INVALID_ID, INVALID_STEP_INDEX, INVALID_COMMAND, INVALID_BRANCH,
    INVALID_SCHEDULE_MODE, INVALID_DATE, INVALID_TIME, INVALID_INTERVAL,
    INVALID_OFFSET, INVALID_DAYS_MASK, INCOMPLETE_SCHEDULE, CONFLICTING_SCHEDULE_FIELDS
};
inline bool isValidAutomationValidationResult(AutomationValidationResult value)
{
    switch(value)
    {
        case AutomationValidationResult::VALID: case AutomationValidationResult::INVALID_ID:
        case AutomationValidationResult::INVALID_STEP_INDEX: case AutomationValidationResult::INVALID_COMMAND:
        case AutomationValidationResult::INVALID_BRANCH: case AutomationValidationResult::INVALID_SCHEDULE_MODE:
        case AutomationValidationResult::INVALID_DATE: case AutomationValidationResult::INVALID_TIME:
        case AutomationValidationResult::INVALID_INTERVAL: case AutomationValidationResult::INVALID_OFFSET:
        case AutomationValidationResult::INVALID_DAYS_MASK: case AutomationValidationResult::INCOMPLETE_SCHEDULE:
        case AutomationValidationResult::CONFLICTING_SCHEDULE_FIELDS: return true;
        default: return false;
    }
}
inline bool isAutomationValid(AutomationValidationResult result)
{ return result==AutomationValidationResult::VALID; }

struct AutomationDate
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    AutomationDate() : year(0),month(0),day(0) {}
    bool isValid() const
    {
        if(year<2000U||year>2199U||month<1U||month>12U||day<1U) return false;
        static const uint8_t daysPerMonth[12]={31,28,31,30,31,30,31,31,30,31,30,31};
        uint8_t maximum=daysPerMonth[month-1U];
        const bool leap=(year%4U==0U)&&((year%100U!=0U)||(year%400U==0U));
        if(month==2U&&leap) maximum=29U;
        return day<=maximum;
    }
    void clear() { *this=AutomationDate{}; }
};

struct AutomationTime
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    AutomationTime() : hour(0xFFU),minute(0xFFU),second(0xFFU) {}
    bool isValid() const { return hour<=23U&&minute<=59U&&second<=59U; }
    void clear() { *this=AutomationTime{}; }
};

#endif
