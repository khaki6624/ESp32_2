#ifndef RULE_ACTION_STEP_H
#define RULE_ACTION_STEP_H

#include <AutomationCommon.h>
#include <AutomationCommand.h>

struct RuleActionStep
{
    AutomationStepIndex stepIndex;
    RuleBranch branch;
    AutomationCommand command;
    // تأخیر پیش از Action این Step است؛ مستقل از command.durationMs باقی می‌ماند
    // و نباید جایگزین یا با Duration فرمان Merge شود.
    uint32_t delayBeforeMs;
    bool enabled;

    RuleActionStep() : stepIndex(INVALID_AUTOMATION_STEP_INDEX),branch(RuleBranch::NONE),
        command{},delayBeforeMs(0),enabled(false) {}
    bool isValid() const { return validate()==AutomationValidationResult::VALID; }
    AutomationValidationResult validate() const
    {
        if(stepIndex==INVALID_AUTOMATION_STEP_INDEX) return AutomationValidationResult::INVALID_STEP_INDEX;
        if(!isValidRuleBranch(branch)||branch==RuleBranch::NONE) return AutomationValidationResult::INVALID_BRANCH;
        if(!command.isValid()) return AutomationValidationResult::INVALID_COMMAND;
        return AutomationValidationResult::VALID;
    }
    bool setCommand(const AutomationCommand& value)
    { if(!value.isValid()) return false;command=value;return true; }
    void clear() { *this=RuleActionStep{}; }
};

#endif
