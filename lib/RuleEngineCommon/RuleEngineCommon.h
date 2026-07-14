#ifndef RULE_ENGINE_COMMON_H
#define RULE_ENGINE_COMMON_H

#include <AutomationModelCommon.h>
#include <ConditionCommon.h>
#include <RequestContext.h>

constexpr size_t RULE_TRIGGER_QUEUE_CAPACITY=8;
constexpr size_t RULE_RUNTIME_STATE_CAPACITY=RULE_MANAGER_CAPACITY;

enum class RuleTriggerType : uint8_t { NONE=0,RISING_EDGE,FALLING_EDGE };
inline bool isValidRuleTriggerType(RuleTriggerType value)
{
    switch(value){case RuleTriggerType::NONE:case RuleTriggerType::RISING_EDGE:case RuleTriggerType::FALLING_EDGE:return true;default:return false;}
}

enum class RuleEngineResult : uint8_t
{
    SUCCESS=0,INVALID_RULE_ID,INVALID_REQUEST_CONTEXT,RULE_NOT_FOUND,RULE_DISABLED,
    RULE_INVALID,EVALUATION_FAILED,TRIGGER_QUEUE_FULL,RUNTIME_STATE_FULL,NOT_FOUND,QUEUE_EMPTY
};
inline bool isValidRuleEngineResult(RuleEngineResult value)
{
    switch(value)
    {
        case RuleEngineResult::SUCCESS:case RuleEngineResult::INVALID_RULE_ID:
        case RuleEngineResult::INVALID_REQUEST_CONTEXT:case RuleEngineResult::RULE_NOT_FOUND:
        case RuleEngineResult::RULE_DISABLED:case RuleEngineResult::RULE_INVALID:
        case RuleEngineResult::EVALUATION_FAILED:case RuleEngineResult::TRIGGER_QUEUE_FULL:
        case RuleEngineResult::RUNTIME_STATE_FULL:case RuleEngineResult::NOT_FOUND:
        case RuleEngineResult::QUEUE_EMPTY:return true;default:return false;
    }
}
inline bool isRuleEngineSuccess(RuleEngineResult value){return value==RuleEngineResult::SUCCESS;}

struct RuleTrigger
{
    RuleId ruleId;RuleBranch branch;RuleTriggerType triggerType;RequestContext request;uint32_t triggeredTimestampMs;
    RuleTrigger():ruleId(INVALID_RULE_ID),branch(RuleBranch::NONE),triggerType(RuleTriggerType::NONE),request{},triggeredTimestampMs(0){}
    bool isValid()const
    {
        if(ruleId==INVALID_RULE_ID||!request.isValid())return false;
        if(triggerType==RuleTriggerType::RISING_EDGE)return branch==RuleBranch::THEN_BRANCH;
        if(triggerType==RuleTriggerType::FALLING_EDGE)return branch==RuleBranch::ELSE_BRANCH;
        return false;
    }
};

struct RuleRuntimeState
{
    RuleId ruleId;ConditionEvaluationResult lastEvaluation;bool initialized;
    uint32_t lastEvaluationTimestampMs;uint32_t lastTransitionTimestampMs;
    RuleRuntimeState():ruleId(INVALID_RULE_ID),lastEvaluation(ConditionEvaluationResult::UNKNOWN),initialized(false),lastEvaluationTimestampMs(0),lastTransitionTimestampMs(0){}
    bool isValid()const
    {
        if(ruleId==INVALID_RULE_ID)return !initialized&&lastEvaluation==ConditionEvaluationResult::UNKNOWN&&lastEvaluationTimestampMs==0U&&lastTransitionTimestampMs==0U;
        return isValidConditionEvaluationResult(lastEvaluation)&&(!initialized||lastEvaluation==ConditionEvaluationResult::TRUE_RESULT||lastEvaluation==ConditionEvaluationResult::FALSE_RESULT);
    }
    void clear(){*this=RuleRuntimeState{};}
};

#endif
