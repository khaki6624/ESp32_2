#ifndef CONDITION_EVALUATOR_H
#define CONDITION_EVALUATOR_H

#include <ConditionExpression.h>
#include <ConditionValueProvider.h>

class ConditionEvaluator
{
public:
    explicit ConditionEvaluator(const ConditionValueProvider& valueProvider);
    ConditionEvaluatorResult evaluate(const ConditionExpression& expression,
                                      ConditionEvaluationResult& output)const;
    ConditionEvaluatorResult evaluateComparison(const ConditionComparison& comparison,
                                                ConditionEvaluationResult& output)const;
private:
    const ConditionValueProvider& valueProvider_;
    ConditionResolveResult resolveOperand(const ConditionOperand& operand,
                                          ConditionResolvedValue& output)const;
    bool compareValues(const ConditionResolvedValue& left,ComparisonOperator comparisonOperator,
                       const ConditionResolvedValue& right,bool& output)const;
};

#endif
