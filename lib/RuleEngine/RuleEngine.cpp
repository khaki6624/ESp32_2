#include "RuleEngine.h"

namespace
{
bool isValidRuleEngineRequest(const RequestContext& request)
{return request.isValid()&&(request.source==CommandSource::RULE_ENGINE||request.source==CommandSource::SYSTEM);}
}

RuleEngine::RuleEngine(const RuleManager& ruleManager,const ConditionEvaluator& evaluator,RuleTriggerQueue& triggerQueue):
    ruleManager_(ruleManager),evaluator_(evaluator),triggerQueue_(triggerQueue),runtimeStates_{},runtimeStateCount_(0),
    nextRuleArrayIndex_(0),lastResult_(RuleEngineResult::SUCCESS){}

RuleRuntimeState* RuleEngine::findRuntimeStateMutable(RuleId ruleId)
{for(size_t i=0;i<runtimeStateCount_;++i)if(runtimeStates_[i].ruleId==ruleId)return &runtimeStates_[i];return nullptr;}
const RuleRuntimeState* RuleEngine::findRuntimeState(RuleId ruleId)const
{for(size_t i=0;i<runtimeStateCount_;++i)if(runtimeStates_[i].ruleId==ruleId)return &runtimeStates_[i];return nullptr;}
RuleRuntimeState* RuleEngine::findOrCreateRuntimeState(RuleId ruleId)
{
    RuleRuntimeState* state=findRuntimeStateMutable(ruleId);if(state!=nullptr)return state;
    if(ruleId==INVALID_RULE_ID||runtimeStateCount_>=RULE_RUNTIME_STATE_CAPACITY)return nullptr;
    runtimeStates_[runtimeStateCount_].ruleId=ruleId;return &runtimeStates_[runtimeStateCount_++];
}

RuleEngineResult RuleEngine::processEvaluation(const Rule& rule,uint32_t nowMs,const RequestContext& request)
{
    RuleRuntimeState* state=findOrCreateRuntimeState(rule.id);
    if(state==nullptr)return RuleEngineResult::RUNTIME_STATE_FULL;
    ConditionEvaluationResult evaluation=ConditionEvaluationResult::ERROR_RESULT;
    if(evaluator_.evaluate(rule.condition,evaluation)!=ConditionEvaluatorResult::SUCCESS)
    {
        state->lastEvaluationTimestampMs=nowMs;
        return RuleEngineResult::EVALUATION_FAILED;
    }
    if(evaluation!=ConditionEvaluationResult::TRUE_RESULT&&evaluation!=ConditionEvaluationResult::FALSE_RESULT)
    {state->lastEvaluationTimestampMs=nowMs;return RuleEngineResult::EVALUATION_FAILED;}
    if(!state->initialized)
    {
        state->initialized=true;state->lastEvaluation=evaluation;state->lastEvaluationTimestampMs=nowMs;
        return RuleEngineResult::SUCCESS;
    }
    if(state->lastEvaluation==evaluation)
    {state->lastEvaluationTimestampMs=nowMs;return RuleEngineResult::SUCCESS;}
    RuleTrigger trigger;trigger.ruleId=rule.id;trigger.request=request;trigger.triggeredTimestampMs=nowMs;
    if(state->lastEvaluation==ConditionEvaluationResult::FALSE_RESULT&&evaluation==ConditionEvaluationResult::TRUE_RESULT)
    {trigger.branch=RuleBranch::THEN_BRANCH;trigger.triggerType=RuleTriggerType::RISING_EDGE;}
    else if(state->lastEvaluation==ConditionEvaluationResult::TRUE_RESULT&&evaluation==ConditionEvaluationResult::FALSE_RESULT)
    {trigger.branch=RuleBranch::ELSE_BRANCH;trigger.triggerType=RuleTriggerType::FALLING_EDGE;}
    else{state->lastEvaluationTimestampMs=nowMs;return RuleEngineResult::EVALUATION_FAILED;}
    const RuleEngineResult queueResult=triggerQueue_.enqueue(trigger);
    if(queueResult!=RuleEngineResult::SUCCESS)return queueResult;
    state->lastEvaluation=evaluation;state->lastEvaluationTimestampMs=nowMs;state->lastTransitionTimestampMs=nowMs;
    return RuleEngineResult::SUCCESS;
}

RuleEngineResult RuleEngine::evaluateRule(RuleId ruleId,uint32_t nowMs,const RequestContext& request)
{
    if(ruleId==INVALID_RULE_ID)return lastResult_=RuleEngineResult::INVALID_RULE_ID;
    if(!isValidRuleEngineRequest(request))return lastResult_=RuleEngineResult::INVALID_REQUEST_CONTEXT;
    const Rule* rule=ruleManager_.findById(ruleId);
    if(rule==nullptr)return lastResult_=RuleEngineResult::RULE_NOT_FOUND;
    if(!rule->isValid())return lastResult_=RuleEngineResult::RULE_INVALID;
    if(!rule->enabled)
    {
        RuleRuntimeState* state=findRuntimeStateMutable(ruleId);
        if(state!=nullptr){*state=RuleRuntimeState{};state->ruleId=ruleId;}
        return lastResult_=RuleEngineResult::RULE_DISABLED;
    }
    return lastResult_=processEvaluation(*rule,nowMs,request);
}

void RuleEngine::update(uint32_t nowMs,const RequestContext& request)
{
    if(!isValidRuleEngineRequest(request)){lastResult_=RuleEngineResult::INVALID_REQUEST_CONTEXT;return;}
    const size_t count=ruleManager_.size();
    if(count==0U){nextRuleArrayIndex_=0U;cleanupRemovedRuleStates();lastResult_=RuleEngineResult::SUCCESS;return;}
    if(nextRuleArrayIndex_>=count)nextRuleArrayIndex_=0U;
    const Rule* rule=ruleManager_.getAt(nextRuleArrayIndex_);
    ++nextRuleArrayIndex_;
    const bool completedCycle=nextRuleArrayIndex_>=count;
    if(completedCycle)nextRuleArrayIndex_=0U;
    if(rule==nullptr)lastResult_=RuleEngineResult::RULE_NOT_FOUND;
    else if(!rule->isValid())lastResult_=RuleEngineResult::RULE_INVALID;
    else if(!rule->enabled)
    {
        RuleRuntimeState* state=findRuntimeStateMutable(rule->id);
        if(state!=nullptr){*state=RuleRuntimeState{};state->ruleId=rule->id;}
        lastResult_=RuleEngineResult::RULE_DISABLED;
    }
    else lastResult_=processEvaluation(*rule,nowMs,request);
    if(completedCycle)cleanupRemovedRuleStates();
}

void RuleEngine::cleanupRemovedRuleStates()
{
    size_t index=0U;
    while(index<runtimeStateCount_)
    {
        if(ruleManager_.findById(runtimeStates_[index].ruleId)!=nullptr){++index;continue;}
        for(size_t i=index+1U;i<runtimeStateCount_;++i)runtimeStates_[i-1U]=runtimeStates_[i];
        --runtimeStateCount_;runtimeStates_[runtimeStateCount_]=RuleRuntimeState{};
    }
}
void RuleEngine::resetRuntimeStates(){for(size_t i=0;i<RULE_RUNTIME_STATE_CAPACITY;++i)runtimeStates_[i]=RuleRuntimeState{};runtimeStateCount_=0U;nextRuleArrayIndex_=0U;lastResult_=RuleEngineResult::SUCCESS;}
size_t RuleEngine::runtimeStateCount()const{return runtimeStateCount_;}size_t RuleEngine::runtimeStateCapacity()const{return RULE_RUNTIME_STATE_CAPACITY;}
size_t RuleEngine::getNextRuleArrayIndex()const{return nextRuleArrayIndex_;}RuleEngineResult RuleEngine::getLastResult()const{return lastResult_;}

// Rule در نسخه فعلی Revision ندارد؛ تغییر Condition با همان RuleId ممکن است نسبت
// به State قبلی Edge بسازد. در آینده باید Revision، Reset-on-Update یا Notification انتخاب شود.
