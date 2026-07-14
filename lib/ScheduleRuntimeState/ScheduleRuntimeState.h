#ifndef SCHEDULE_RUNTIME_STATE_H
#define SCHEDULE_RUNTIME_STATE_H

#include <AutomationCommon.h>

struct ScheduleRuntimeState
{
    ScheduleId scheduleId;
    AutomationStepIndex commandIndex;
    bool initialized;
    AutomationDate lastExecutedDate;
    uint32_t lastExecutedMonotonicMs;
    bool fixedDateTimeCompleted;
    uint32_t commandFingerprint;
    ScheduleRuntimeState() : scheduleId(INVALID_SCHEDULE_ID),
        commandIndex(INVALID_AUTOMATION_STEP_INDEX), initialized(false), lastExecutedDate{},
        lastExecutedMonotonicMs(0U), fixedDateTimeCompleted(false), commandFingerprint(0U) {}
    bool isValid() const
    {
        if (scheduleId == INVALID_SCHEDULE_ID || commandIndex == INVALID_AUTOMATION_STEP_INDEX)
            return scheduleId == INVALID_SCHEDULE_ID &&
                commandIndex == INVALID_AUTOMATION_STEP_INDEX && !initialized &&
                lastExecutedDate.isEmpty() && lastExecutedMonotonicMs == 0U &&
                !fixedDateTimeCompleted && commandFingerprint == 0U;
        return (lastExecutedDate.isEmpty() || lastExecutedDate.isValid()) &&
            (!initialized || commandFingerprint != 0U);
    }
    void clear() { *this = ScheduleRuntimeState{}; }
};

#endif
