#ifndef CONDITION_COMMON_H
#define CONDITION_COMMON_H

#include <stddef.h>
#include <stdint.h>

constexpr size_t CONDITION_MAX_COMPARISONS = 4;

enum class ConditionSourceType : uint8_t
{
    NONE = 0, INPUT, ADC, DEVICE, ENDPOINT, NODE, SYSTEM, CONSTANT
};
inline bool isValidConditionSourceType(ConditionSourceType value)
{
    switch(value)
    {
        case ConditionSourceType::NONE: case ConditionSourceType::INPUT:
        case ConditionSourceType::ADC: case ConditionSourceType::DEVICE:
        case ConditionSourceType::ENDPOINT: case ConditionSourceType::NODE:
        case ConditionSourceType::SYSTEM: case ConditionSourceType::CONSTANT: return true;
        default: return false;
    }
}

enum class ConditionValueType : uint8_t
{
    NONE = 0, BOOLEAN, INTEGER, FLOAT, PERCENTAGE, ENUM_VALUE
};
inline bool isValidConditionValueType(ConditionValueType value)
{
    switch(value)
    {
        case ConditionValueType::NONE: case ConditionValueType::BOOLEAN:
        case ConditionValueType::INTEGER: case ConditionValueType::FLOAT:
        case ConditionValueType::PERCENTAGE: case ConditionValueType::ENUM_VALUE: return true;
        default: return false;
    }
}

enum class ComparisonOperator : uint8_t
{
    NONE = 0, EQUAL, NOT_EQUAL, LESS_THAN, LESS_THAN_OR_EQUAL,
    GREATER_THAN, GREATER_THAN_OR_EQUAL
};
inline bool isValidComparisonOperator(ComparisonOperator value)
{
    switch(value)
    {
        case ComparisonOperator::NONE: case ComparisonOperator::EQUAL:
        case ComparisonOperator::NOT_EQUAL: case ComparisonOperator::LESS_THAN:
        case ComparisonOperator::LESS_THAN_OR_EQUAL: case ComparisonOperator::GREATER_THAN:
        case ComparisonOperator::GREATER_THAN_OR_EQUAL: return true;
        default: return false;
    }
}

enum class LogicalOperator : uint8_t { NONE = 0, AND, OR };
inline bool isValidLogicalOperator(LogicalOperator value)
{
    switch(value)
    {
        case LogicalOperator::NONE: case LogicalOperator::AND: case LogicalOperator::OR: return true;
        default: return false;
    }
}

enum class ConditionEvaluationResult : uint8_t
{ UNKNOWN = 0, FALSE_RESULT, TRUE_RESULT, ERROR_RESULT };
inline bool isValidConditionEvaluationResult(ConditionEvaluationResult value)
{
    switch(value)
    {
        case ConditionEvaluationResult::UNKNOWN: case ConditionEvaluationResult::FALSE_RESULT:
        case ConditionEvaluationResult::TRUE_RESULT: case ConditionEvaluationResult::ERROR_RESULT: return true;
        default: return false;
    }
}

enum class ConditionValidationResult : uint8_t
{
    VALID = 0, INVALID_OPERAND, INVALID_SOURCE, INVALID_VALUE_TYPE,
    INVALID_OPERATOR, TYPE_MISMATCH, TOO_MANY_COMPARISONS,
    INVALID_LOGICAL_OPERATOR, EMPTY_EXPRESSION,
    EXPRESSION_ALREADY_INITIALIZED
};
inline bool isValidConditionValidationResult(ConditionValidationResult value)
{
    switch(value)
    {
        case ConditionValidationResult::VALID: case ConditionValidationResult::INVALID_OPERAND:
        case ConditionValidationResult::INVALID_SOURCE: case ConditionValidationResult::INVALID_VALUE_TYPE:
        case ConditionValidationResult::INVALID_OPERATOR: case ConditionValidationResult::TYPE_MISMATCH:
        case ConditionValidationResult::TOO_MANY_COMPARISONS:
        case ConditionValidationResult::INVALID_LOGICAL_OPERATOR:
        case ConditionValidationResult::EMPTY_EXPRESSION:
        case ConditionValidationResult::EXPRESSION_ALREADY_INITIALIZED: return true;
        default: return false;
    }
}
inline bool isConditionValid(ConditionValidationResult result)
{ return result == ConditionValidationResult::VALID; }

#endif
