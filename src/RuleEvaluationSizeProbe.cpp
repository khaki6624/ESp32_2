#include <RuleEngine.h>

extern "C"
{
unsigned char conditionResolvedValueSizeProbe[sizeof(ConditionResolvedValue)];
unsigned char conditionEvaluatorSizeProbe[sizeof(ConditionEvaluator)];
unsigned char ruleTriggerSizeProbe[sizeof(RuleTrigger)];
unsigned char ruleRuntimeStateSizeProbe[sizeof(RuleRuntimeState)];
unsigned char ruleTriggerQueueSizeProbe[sizeof(RuleTriggerQueue)];
unsigned char ruleEngineSizeProbe[sizeof(RuleEngine)];
}
