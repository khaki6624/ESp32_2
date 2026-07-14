#ifndef CONDITION_EVALUATION_COMMON_H
#define CONDITION_EVALUATION_COMMON_H

#include <math.h>
#include <ConditionCommon.h>

enum class ConditionResolveResult : uint8_t
{
    SUCCESS = 0, INVALID_OPERAND, UNSUPPORTED_SOURCE, SOURCE_NOT_FOUND,
    SOURCE_DISABLED, SOURCE_OFFLINE, VALUE_UNAVAILABLE, VALUE_TYPE_MISMATCH,
    PROVIDER_ERROR
};

inline bool isValidConditionResolveResult(ConditionResolveResult result)
{
    switch(result)
    {
        case ConditionResolveResult::SUCCESS: case ConditionResolveResult::INVALID_OPERAND:
        case ConditionResolveResult::UNSUPPORTED_SOURCE: case ConditionResolveResult::SOURCE_NOT_FOUND:
        case ConditionResolveResult::SOURCE_DISABLED: case ConditionResolveResult::SOURCE_OFFLINE:
        case ConditionResolveResult::VALUE_UNAVAILABLE: case ConditionResolveResult::VALUE_TYPE_MISMATCH:
        case ConditionResolveResult::PROVIDER_ERROR:return true;
        default:return false;
    }
}
inline bool isConditionResolveSuccess(ConditionResolveResult result)
{return result==ConditionResolveResult::SUCCESS;}

enum class ConditionEvaluatorResult : uint8_t
{
    SUCCESS = 0, INVALID_EXPRESSION, INVALID_COMPARISON, RESOLVE_FAILED,
    TYPE_MISMATCH, UNSUPPORTED_OPERATOR, NON_FINITE_VALUE
};

inline bool isValidConditionEvaluatorResult(ConditionEvaluatorResult result)
{
    switch(result)
    {
        case ConditionEvaluatorResult::SUCCESS: case ConditionEvaluatorResult::INVALID_EXPRESSION:
        case ConditionEvaluatorResult::INVALID_COMPARISON: case ConditionEvaluatorResult::RESOLVE_FAILED:
        case ConditionEvaluatorResult::TYPE_MISMATCH: case ConditionEvaluatorResult::UNSUPPORTED_OPERATOR:
        case ConditionEvaluatorResult::NON_FINITE_VALUE:return true;
        default:return false;
    }
}
inline bool isConditionEvaluatorSuccess(ConditionEvaluatorResult result)
{return result==ConditionEvaluatorResult::SUCCESS;}

struct ConditionResolvedValue
{
    ConditionValueType type;
    union
    {
        bool booleanValue; int32_t integerValue; float floatValue;
        uint8_t percentageValue; int32_t enumValue;
    };
    bool valid;

    ConditionResolvedValue():type(ConditionValueType::NONE),integerValue(0),valid(false){}
    void clear(){*this=ConditionResolvedValue{};}
    static ConditionResolvedValue makeBoolean(bool value)
    {ConditionResolvedValue r;r.type=ConditionValueType::BOOLEAN;r.booleanValue=value;r.valid=true;return r;}
    static ConditionResolvedValue makeInteger(int32_t value)
    {ConditionResolvedValue r;r.type=ConditionValueType::INTEGER;r.integerValue=value;r.valid=true;return r;}
    static ConditionResolvedValue makeFloat(float value)
    {ConditionResolvedValue r;if(isfinite(value)){r.type=ConditionValueType::FLOAT;r.floatValue=value;r.valid=true;}return r;}
    static ConditionResolvedValue makePercentage(int32_t value)
    {ConditionResolvedValue r;if(value>=0&&value<=100){r.type=ConditionValueType::PERCENTAGE;r.percentageValue=static_cast<uint8_t>(value);r.valid=true;}return r;}
    static ConditionResolvedValue makeEnum(int32_t value)
    {ConditionResolvedValue r;r.type=ConditionValueType::ENUM_VALUE;r.enumValue=value;r.valid=true;return r;}
    bool isValid()const
    {
        if(!valid||!isValidConditionValueType(type)||type==ConditionValueType::NONE)return false;
        if(type==ConditionValueType::FLOAT&&!isfinite(floatValue))return false;
        return type!=ConditionValueType::PERCENTAGE||percentageValue<=100U;
    }
};

#endif
