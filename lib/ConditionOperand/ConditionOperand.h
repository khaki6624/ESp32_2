#ifndef CONDITION_OPERAND_H
#define CONDITION_OPERAND_H

#include <math.h>
#include <ConditionCommon.h>

struct ConditionOperand
{
    // sourceId و subIndex فقط شناسه منطقی Reference هستند؛ تفسیر آن‌ها بر عهده
    // Adapter یا Evaluator آینده است و این مدل هیچ Registry Lookup یا Driver Read ندارد.
    ConditionSourceType sourceType;
    uint32_t sourceId;
    uint16_t subIndex;
    ConditionValueType valueType;
    union
    {
        bool booleanValue;
        int32_t integerValue;
        float floatValue;
        uint8_t percentageValue;
        int32_t enumValue;
    };
    bool isReference;
    bool valid;

    ConditionOperand() : sourceType(ConditionSourceType::NONE), sourceId(0), subIndex(0),
        valueType(ConditionValueType::NONE), integerValue(0), isReference(false), valid(false) {}

    void clear() { *this = ConditionOperand{}; }
    void invalidate() { clear(); }

    static ConditionOperand makeReference(ConditionSourceType source, uint32_t id,
                                          ConditionValueType type, uint16_t sourceSubIndex = 0)
    {
        ConditionOperand result;
        if (!isValidConditionSourceType(source) || source == ConditionSourceType::NONE ||
            source == ConditionSourceType::CONSTANT || id == 0 ||
            !isValidConditionValueType(type) || type == ConditionValueType::NONE) return result;
        result.sourceType = source; result.sourceId = id; result.subIndex = sourceSubIndex;
        result.valueType = type; result.isReference = true; result.valid = true;
        return result;
    }

    static ConditionOperand makeBoolean(bool value)
    { ConditionOperand r; r.sourceType=ConditionSourceType::CONSTANT;r.valueType=ConditionValueType::BOOLEAN;r.booleanValue=value;r.valid=true;return r; }
    static ConditionOperand makeInteger(int32_t value)
    { ConditionOperand r;r.sourceType=ConditionSourceType::CONSTANT;r.valueType=ConditionValueType::INTEGER;r.integerValue=value;r.valid=true;return r; }
    static ConditionOperand makeFloat(float value)
    { ConditionOperand r;if(isfinite(value)){r.sourceType=ConditionSourceType::CONSTANT;r.valueType=ConditionValueType::FLOAT;r.floatValue=value;r.valid=true;}return r; }
    static ConditionOperand makePercentage(int32_t value)
    { ConditionOperand r;if(value>=0&&value<=100){r.sourceType=ConditionSourceType::CONSTANT;r.valueType=ConditionValueType::PERCENTAGE;r.percentageValue=static_cast<uint8_t>(value);r.valid=true;}return r; }
    static ConditionOperand makeEnum(int32_t value)
    { ConditionOperand r;r.sourceType=ConditionSourceType::CONSTANT;r.valueType=ConditionValueType::ENUM_VALUE;r.enumValue=value;r.valid=true;return r; }

    bool isValid() const
    {
        if(!valid || !isValidConditionSourceType(sourceType) ||
           !isValidConditionValueType(valueType) || valueType==ConditionValueType::NONE) return false;
        if(isReference)
            return sourceType!=ConditionSourceType::NONE && sourceType!=ConditionSourceType::CONSTANT && sourceId!=0;
        if(sourceType!=ConditionSourceType::CONSTANT || sourceId!=0 || subIndex!=0) return false;
        if(valueType==ConditionValueType::FLOAT && !isfinite(floatValue)) return false;
        if(valueType==ConditionValueType::PERCENTAGE && percentageValue>100U) return false;
        return true;
    }
    bool isConstant() const { return isValid() && !isReference && sourceType==ConditionSourceType::CONSTANT; }
    bool isSourceReference() const { return isValid() && isReference; }
    bool getBoolean(bool& out) const { if(!isConstant()||valueType!=ConditionValueType::BOOLEAN)return false;out=booleanValue;return true; }
    bool getInteger(int32_t& out) const { if(!isConstant()||valueType!=ConditionValueType::INTEGER)return false;out=integerValue;return true; }
    bool getFloat(float& out) const { if(!isConstant()||valueType!=ConditionValueType::FLOAT)return false;out=floatValue;return true; }
    bool getPercentage(uint8_t& out) const { if(!isConstant()||valueType!=ConditionValueType::PERCENTAGE)return false;out=percentageValue;return true; }
    bool getEnum(int32_t& out) const { if(!isConstant()||valueType!=ConditionValueType::ENUM_VALUE)return false;out=enumValue;return true; }
};

#endif
