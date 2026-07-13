#ifndef SCHEDULED_COMMAND_H
#define SCHEDULED_COMMAND_H

#include <AutomationCommon.h>
#include <AutomationCommand.h>

// نسخه اول Command و Trigger زمان‌بندی را برای سادگی در یک Value Object نگه می‌دارد
// و هیچ اجرای زمان‌بندی ندارد. در آینده می‌توان Trigger و Action را بدون انتقال
// منطق Scheduler به این مدل، به ScheduleTrigger و ScheduledAction تفکیک کرد.
struct ScheduledCommand
{
    ScheduleId scheduleId;
    AutomationStepIndex commandIndex;
    AutomationCommand command;
    ScheduleMode mode;
    AutomationDate date;
    AutomationTime time;
    uint32_t intervalMs;
    int32_t solarOffsetMinutes;
    ScheduleDaysMask daysMask;
    bool enabled;

    ScheduledCommand() : scheduleId(INVALID_SCHEDULE_ID),commandIndex(INVALID_AUTOMATION_STEP_INDEX),
        command{},mode(ScheduleMode::NONE),date{},time{},intervalMs(0),solarOffsetMinutes(0),
        daysMask(0),enabled(false) {}
    bool isValid() const { return validate()==AutomationValidationResult::VALID; }
    AutomationValidationResult validate() const
    {
        if(scheduleId==INVALID_SCHEDULE_ID) return AutomationValidationResult::INVALID_ID;
        if(commandIndex==INVALID_AUTOMATION_STEP_INDEX) return AutomationValidationResult::INVALID_STEP_INDEX;
        if(!command.isValid()) return AutomationValidationResult::INVALID_COMMAND;
        if(!isValidScheduleMode(mode)||mode==ScheduleMode::NONE) return AutomationValidationResult::INVALID_SCHEDULE_MODE;

        switch(mode)
        {
            case ScheduleMode::FIXED_TIME:
                if(!date.isEmpty()||intervalMs!=0U||solarOffsetMinutes!=0) return AutomationValidationResult::CONFLICTING_SCHEDULE_FIELDS;
                if(time.isEmpty()||daysMask==0U) return AutomationValidationResult::INCOMPLETE_SCHEDULE;
                if(!time.isValid()) return AutomationValidationResult::INVALID_TIME;
                if(!isValidScheduleDaysMask(daysMask)) return AutomationValidationResult::INVALID_DAYS_MASK;
                return AutomationValidationResult::VALID;
            case ScheduleMode::FIXED_DATE_TIME:
                if(daysMask!=0U||intervalMs!=0U||solarOffsetMinutes!=0) return AutomationValidationResult::CONFLICTING_SCHEDULE_FIELDS;
                if(date.isEmpty()||time.isEmpty()) return AutomationValidationResult::INCOMPLETE_SCHEDULE;
                if(!date.isValid()) return AutomationValidationResult::INVALID_DATE;
                if(!time.isValid()) return AutomationValidationResult::INVALID_TIME;
                return AutomationValidationResult::VALID;
            case ScheduleMode::INTERVAL:
                if(!date.isEmpty()||!time.isEmpty()||daysMask!=0U||solarOffsetMinutes!=0) return AutomationValidationResult::CONFLICTING_SCHEDULE_FIELDS;
                if(intervalMs==0U) return AutomationValidationResult::INVALID_INTERVAL;
                return AutomationValidationResult::VALID;
            case ScheduleMode::SUNRISE_OFFSET:
            case ScheduleMode::SUNSET_OFFSET:
                if(!date.isEmpty()||!time.isEmpty()||intervalMs!=0U) return AutomationValidationResult::CONFLICTING_SCHEDULE_FIELDS;
                if(daysMask==0U) return AutomationValidationResult::INCOMPLETE_SCHEDULE;
                if(!isValidScheduleDaysMask(daysMask)) return AutomationValidationResult::INVALID_DAYS_MASK;
                if(solarOffsetMinutes < -720 || solarOffsetMinutes > 720) return AutomationValidationResult::INVALID_OFFSET;
                return AutomationValidationResult::VALID;
            case ScheduleMode::MANUAL:
                if(!date.isEmpty()||!time.isEmpty()||intervalMs!=0U||solarOffsetMinutes!=0||daysMask!=0U)
                    return AutomationValidationResult::CONFLICTING_SCHEDULE_FIELDS;
                return AutomationValidationResult::VALID;
            case ScheduleMode::NONE:
            default: return AutomationValidationResult::INVALID_SCHEDULE_MODE;
        }
    }
    bool setCommand(const AutomationCommand& value)
    { if(!value.isValid()) return false;command=value;return true; }
    void clear() { *this=ScheduledCommand{}; }
};

#endif
