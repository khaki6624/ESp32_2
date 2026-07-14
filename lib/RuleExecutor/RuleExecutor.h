#ifndef RULE_EXECUTOR_H
#define RULE_EXECUTOR_H

#include <AutomationCommandFactory.h>
#include <CommandIdProvider.h>
#include <RuleManager.h>
#include <RuleTriggerQueue.h>
#include <RuntimeCommandSink.h>

class RuleExecutor
{
public:
    RuleExecutor(const RuleManager& ruleManager, RuleTriggerQueue& triggerQueue,
        const AutomationCommandFactory& commandFactory, CommandIdProvider& commandIdProvider,
        RuntimeCommandSink& commandSink);

    void update(uint32_t nowMs);
    RuleExecutionResult cancelCurrent();
    void reset();
    RuleExecutionState getState() const;
    RuleExecutionResult getLastResult() const;
    bool isRunning() const;
    RuleId getCurrentRuleId() const;
    RuleBranch getCurrentBranch() const;
    AutomationStepIndex getCurrentStepIndex() const;

private:
    const RuleManager& ruleManager_;
    RuleTriggerQueue& triggerQueue_;
    const AutomationCommandFactory& commandFactory_;
    CommandIdProvider& commandIdProvider_;
    RuntimeCommandSink& commandSink_;
    RuleExecutionState state_;
    RuleExecutionResult lastResult_;
    RuleTrigger currentTrigger_;
    size_t currentActionArrayIndex_;
    CommandId firstReservedCommandId_;
    CommandId nextCommandId_;
    size_t activeCommandCount_;
    uint32_t delayStartedMs_;
    uint32_t delayDurationMs_;

    bool loadNextTrigger();
    RuleExecutionResult validateCurrentRule(const Rule*& outputRule) const;
    size_t countEnabledActions(const Rule& rule, RuleBranch branch) const;
    const RuleActionStep* findNextEnabledAction(const Rule& rule, RuleBranch branch,
        size_t startIndex, size_t& foundIndex) const;
    RuleExecutionResult reserveCommandIds(const Rule& rule);
    RuleExecutionResult processReadyAction(uint32_t nowMs);
    RuleExecutionResult submitCurrentAction(uint32_t nowMs);
    void finishCurrent(RuleExecutionState terminalState, RuleExecutionResult result);
    void clearCurrentRuntime();
};

#endif
