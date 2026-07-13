#ifndef CONDITION_COMPARISON_H
#define CONDITION_COMPARISON_H

#include <ConditionOperand.h>

struct ConditionComparison
{
    // نسخه اول Strict Type Matching دارد؛ هیچ Conversion خودکاری بین INTEGER،
    // FLOAT و PERCENTAGE انجام نمی‌شود و Normalization به لایه‌های آینده واگذار شده است.
    ConditionOperand left;
    ComparisonOperator comparisonOperator;
    ConditionOperand right;

    ConditionComparison() : left{}, comparisonOperator(ComparisonOperator::NONE), right{} {}
    bool isValid() const { return validate() == ConditionValidationResult::VALID; }
    ConditionValidationResult validate() const
    {
        if(!isValidConditionSourceType(left.sourceType) || !isValidConditionSourceType(right.sourceType))
            return ConditionValidationResult::INVALID_SOURCE;
        if(!isValidConditionValueType(left.valueType) || !isValidConditionValueType(right.valueType) ||
           left.valueType==ConditionValueType::NONE || right.valueType==ConditionValueType::NONE)
            return ConditionValidationResult::INVALID_VALUE_TYPE;
        if(!left.isValid() || !right.isValid()) return ConditionValidationResult::INVALID_OPERAND;
        if(!isValidComparisonOperator(comparisonOperator) || comparisonOperator==ComparisonOperator::NONE)
            return ConditionValidationResult::INVALID_OPERATOR;
        if(left.valueType!=right.valueType) return ConditionValidationResult::TYPE_MISMATCH;
        if((left.valueType==ConditionValueType::BOOLEAN || left.valueType==ConditionValueType::ENUM_VALUE) &&
           comparisonOperator!=ComparisonOperator::EQUAL && comparisonOperator!=ComparisonOperator::NOT_EQUAL)
            return ConditionValidationResult::INVALID_OPERATOR;
        return ConditionValidationResult::VALID;
    }
};

#endif
