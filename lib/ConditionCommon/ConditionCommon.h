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
{ return static_cast<uint8_t>(value) <= static_cast<uint8_t>(ConditionSourceType::CONSTANT); }

enum class ConditionValueType : uint8_t
{
    NONE = 0, BOOLEAN, INTEGER, FLOAT, PERCENTAGE, ENUM_VALUE
};
inline bool isValidConditionValueType(ConditionValueType value)
{ return static_cast<uint8_t>(value) <= static_cast<uint8_t>(ConditionValueType::ENUM_VALUE); }

enum class ComparisonOperator : uint8_t
{
    NONE = 0, EQUAL, NOT_EQUAL, LESS_THAN, LESS_THAN_OR_EQUAL,
    GREATER_THAN, GREATER_THAN_OR_EQUAL
};
inline bool isValidComparisonOperator(ComparisonOperator value)
{ return static_cast<uint8_t>(value) <= static_cast<uint8_t>(ComparisonOperator::GREATER_THAN_OR_EQUAL); }

enum class LogicalOperator : uint8_t { NONE = 0, AND, OR };
inline bool isValidLogicalOperator(LogicalOperator value)
{ return static_cast<uint8_t>(value) <= static_cast<uint8_t>(LogicalOperator::OR); }

enum class ConditionEvaluationResult : uint8_t
{ UNKNOWN = 0, FALSE_RESULT, TRUE_RESULT, ERROR_RESULT };
inline bool isValidConditionEvaluationResult(ConditionEvaluationResult value)
{ return static_cast<uint8_t>(value) <= static_cast<uint8_t>(ConditionEvaluationResult::ERROR_RESULT); }

enum class ConditionValidationResult : uint8_t
{
    VALID = 0, INVALID_OPERAND, INVALID_SOURCE, INVALID_VALUE_TYPE,
    INVALID_OPERATOR, TYPE_MISMATCH, TOO_MANY_COMPARISONS,
    INVALID_LOGICAL_OPERATOR, EMPTY_EXPRESSION
};
inline bool isValidConditionValidationResult(ConditionValidationResult value)
{ return static_cast<uint8_t>(value) <= static_cast<uint8_t>(ConditionValidationResult::EMPTY_EXPRESSION); }
inline bool isConditionValid(ConditionValidationResult result)
{ return result == ConditionValidationResult::VALID; }

#endif
