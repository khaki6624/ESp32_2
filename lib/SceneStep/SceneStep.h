#ifndef SCENE_STEP_H
#define SCENE_STEP_H

#include <AutomationCommon.h>
#include <Command.h>

struct SceneStep
{
    AutomationStepIndex stepIndex;
    Command command;
    // فاصله پس از این Step و پیش از Step بعدی است؛ با Command.durationMs که مدت
    // Action موقت و Restore را بیان می‌کند متفاوت است و نباید با آن ادغام شود.
    uint32_t delayAfterMs;
    bool enabled;

    SceneStep() : stepIndex(INVALID_AUTOMATION_STEP_INDEX),command{},delayAfterMs(0),enabled(false) {}
    bool isValid() const { return validate()==AutomationValidationResult::VALID; }
    AutomationValidationResult validate() const
    {
        if(stepIndex==INVALID_AUTOMATION_STEP_INDEX) return AutomationValidationResult::INVALID_STEP_INDEX;
        if(!command.isValid()) return AutomationValidationResult::INVALID_COMMAND;
        return AutomationValidationResult::VALID;
    }
    bool setCommand(const Command& value)
    { if(!value.isValid()) return false;command=value;return true; }
    void clear() { *this=SceneStep{}; }
};

#endif
