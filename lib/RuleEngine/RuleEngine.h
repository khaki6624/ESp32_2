#ifndef RULE_ENGINE_H
#define RULE_ENGINE_H

#include <ConditionEvaluator.h>
#include <RuleManager.h>
#include <RuleTriggerQueue.h>

class RuleEngine
{
public:
    RuleEngine(const RuleManager& ruleManager,const ConditionEvaluator& evaluator,RuleTriggerQueue& triggerQueue);
    void update(uint32_t nowMs,const RequestContext& request);
    RuleEngineResult evaluateRule(RuleId ruleId,uint32_t nowMs,const RequestContext& request);
    void resetRuntimeStates();size_t runtimeStateCount()const;size_t runtimeStateCapacity()const;
    size_t getNextRuleArrayIndex()const;const RuleRuntimeState* findRuntimeState(RuleId ruleId)const;
    RuleEngineResult getLastResult()const;
private:
    const RuleManager& ruleManager_;const ConditionEvaluator& evaluator_;RuleTriggerQueue& triggerQueue_;
    RuleRuntimeState runtimeStates_[RULE_RUNTIME_STATE_CAPACITY];size_t runtimeStateCount_;
    size_t nextRuleArrayIndex_;RuleEngineResult lastResult_;
    RuleRuntimeState* findRuntimeStateMutable(RuleId ruleId);
    RuleRuntimeState* findOrCreateRuntimeState(RuleId ruleId);
    RuleEngineResult processEvaluation(const Rule& rule,uint32_t nowMs,const RequestContext& request);
    void cleanupRemovedRuleStates();
};

#endif
