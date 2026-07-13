#ifndef CONDITION_EXPRESSION_H
#define CONDITION_EXPRESSION_H

#include <ConditionComparison.h>

// نسخه اول Expression تخت است؛ اولویت عملگر و پرانتز را Evaluator آینده تعیین می‌کند.
struct ConditionExpression
{
    ConditionComparison comparisons[CONDITION_MAX_COMPARISONS];
    LogicalOperator operators[CONDITION_MAX_COMPARISONS - 1U];
    uint8_t comparisonCount;

    ConditionExpression() : comparisons{}, operators{}, comparisonCount(0) {}
    void clear()
    {
        for(size_t i=0;i<CONDITION_MAX_COMPARISONS;++i) comparisons[i]=ConditionComparison{};
        for(size_t i=0;i<CONDITION_MAX_COMPARISONS-1U;++i) operators[i]=LogicalOperator::NONE;
        comparisonCount=0;
    }
    bool isValid() const
    {
        if(comparisonCount==0 || comparisonCount>CONDITION_MAX_COMPARISONS) return false;
        for(size_t i=0;i<comparisonCount;++i) if(!comparisons[i].isValid()) return false;
        for(size_t i=0;i+1U<comparisonCount;++i)
            if(operators[i]!=LogicalOperator::AND && operators[i]!=LogicalOperator::OR) return false;
        return true;
    }
    bool isEmpty() const { return comparisonCount==0; }
    bool isFull() const { return comparisonCount>=CONDITION_MAX_COMPARISONS; }
    ConditionValidationResult addFirst(const ConditionComparison& comparison)
    {
        if(!isEmpty()) return ConditionValidationResult::TOO_MANY_COMPARISONS;
        const ConditionValidationResult result=comparison.validate();
        if(result!=ConditionValidationResult::VALID) return result;
        comparisons[0]=comparison;comparisonCount=1;return ConditionValidationResult::VALID;
    }
    ConditionValidationResult add(LogicalOperator logicalOperator,const ConditionComparison& comparison)
    {
        if(isEmpty()) return ConditionValidationResult::EMPTY_EXPRESSION;
        if(isFull()) return ConditionValidationResult::TOO_MANY_COMPARISONS;
        if(!isValidLogicalOperator(logicalOperator) || logicalOperator==LogicalOperator::NONE)
            return ConditionValidationResult::INVALID_LOGICAL_OPERATOR;
        const ConditionValidationResult result=comparison.validate();
        if(result!=ConditionValidationResult::VALID) return result;
        operators[comparisonCount-1U]=logicalOperator;
        comparisons[comparisonCount]=comparison;
        ++comparisonCount;
        return ConditionValidationResult::VALID;
    }
    const ConditionComparison* getComparisonAt(size_t index) const
    { return index<comparisonCount?&comparisons[index]:nullptr; }
    LogicalOperator getOperatorBefore(size_t comparisonIndex) const
    {
        if(comparisonIndex==0 || comparisonIndex>=comparisonCount) return LogicalOperator::NONE;
        return operators[comparisonIndex-1U];
    }
};

#endif
