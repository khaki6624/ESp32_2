#ifndef AUTOMATION_COMMAND_COMMON_H
#define AUTOMATION_COMMAND_COMMON_H

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <CommandCommon.h>

constexpr size_t AUTOMATION_COMMAND_MAX_ARGUMENTS = 2;

enum class AutomationArgumentType : uint8_t
{
    NONE = 0, BOOLEAN, INTEGER, FLOAT, PERCENTAGE, DURATION_MS, IDENTIFIER, ENUM_VALUE
};

inline bool isValidAutomationArgumentType(AutomationArgumentType value)
{
    switch (value)
    {
        case AutomationArgumentType::NONE: case AutomationArgumentType::BOOLEAN:
        case AutomationArgumentType::INTEGER: case AutomationArgumentType::FLOAT:
        case AutomationArgumentType::PERCENTAGE: case AutomationArgumentType::DURATION_MS:
        case AutomationArgumentType::IDENTIFIER: case AutomationArgumentType::ENUM_VALUE:
            return true;
        default: return false;
    }
}

enum class AutomationCommandValidationResult : uint8_t
{
    VALID = 0, INVALID_DOMAIN, INVALID_DOMAIN_INDEX, INVALID_OPERATION,
    INVALID_ARGUMENT, TOO_MANY_ARGUMENTS, INVALID_DURATION,
    QUERY_NOT_SUPPORTED, TEXT_ARGUMENT_NOT_SUPPORTED,
    DANGEROUS_OPERATION_NOT_PERSISTABLE, DOMAIN_OPERATION_MISMATCH
};

inline bool isValidAutomationCommandValidationResult(AutomationCommandValidationResult value)
{
    switch (value)
    {
        case AutomationCommandValidationResult::VALID:
        case AutomationCommandValidationResult::INVALID_DOMAIN:
        case AutomationCommandValidationResult::INVALID_DOMAIN_INDEX:
        case AutomationCommandValidationResult::INVALID_OPERATION:
        case AutomationCommandValidationResult::INVALID_ARGUMENT:
        case AutomationCommandValidationResult::TOO_MANY_ARGUMENTS:
        case AutomationCommandValidationResult::INVALID_DURATION:
        case AutomationCommandValidationResult::QUERY_NOT_SUPPORTED:
        case AutomationCommandValidationResult::TEXT_ARGUMENT_NOT_SUPPORTED:
        case AutomationCommandValidationResult::DANGEROUS_OPERATION_NOT_PERSISTABLE:
        case AutomationCommandValidationResult::DOMAIN_OPERATION_MISMATCH:
            return true;
        default: return false;
    }
}

inline bool isAutomationCommandValid(AutomationCommandValidationResult value)
{ return value == AutomationCommandValidationResult::VALID; }

struct AutomationCommandArgument
{
    AutomationArgumentType type;
    union
    {
        bool booleanValue;
        int32_t integerValue;
        float floatValue;
        uint8_t percentageValue;
        uint32_t durationMs;
        uint32_t identifierValue;
        int32_t enumValue;
    };
    bool valid;

    AutomationCommandArgument() : type(AutomationArgumentType::NONE), integerValue(0), valid(false) {}
    void clear() { *this = AutomationCommandArgument{}; }
    void invalidate() { clear(); }
    static AutomationCommandArgument makeBoolean(bool value)
    { AutomationCommandArgument r; r.type=AutomationArgumentType::BOOLEAN;r.booleanValue=value;r.valid=true;return r; }
    static AutomationCommandArgument makeInteger(int32_t value)
    { AutomationCommandArgument r;r.type=AutomationArgumentType::INTEGER;r.integerValue=value;r.valid=true;return r; }
    static AutomationCommandArgument makeFloat(float value)
    { AutomationCommandArgument r;if(isfinite(value)){r.type=AutomationArgumentType::FLOAT;r.floatValue=value;r.valid=true;}return r; }
    static AutomationCommandArgument makePercentage(int32_t value)
    { AutomationCommandArgument r;if(value>=0&&value<=100){r.type=AutomationArgumentType::PERCENTAGE;r.percentageValue=static_cast<uint8_t>(value);r.valid=true;}return r; }
    static AutomationCommandArgument makeDuration(uint32_t value)
    { AutomationCommandArgument r;r.type=AutomationArgumentType::DURATION_MS;r.durationMs=value;r.valid=true;return r; }
    static AutomationCommandArgument makeIdentifier(uint32_t value)
    { AutomationCommandArgument r;r.type=AutomationArgumentType::IDENTIFIER;r.identifierValue=value;r.valid=true;return r; }
    static AutomationCommandArgument makeEnum(int32_t value)
    { AutomationCommandArgument r;r.type=AutomationArgumentType::ENUM_VALUE;r.enumValue=value;r.valid=true;return r; }
    bool isValid() const
    {
        if(!valid||type==AutomationArgumentType::NONE||!isValidAutomationArgumentType(type))return false;
        if(type==AutomationArgumentType::FLOAT&&!isfinite(floatValue))return false;
        return type!=AutomationArgumentType::PERCENTAGE||percentageValue<=100U;
    }
    bool getBoolean(bool& out)const{if(!isValid()||type!=AutomationArgumentType::BOOLEAN)return false;out=booleanValue;return true;}
    bool getInteger(int32_t& out)const{if(!isValid()||type!=AutomationArgumentType::INTEGER)return false;out=integerValue;return true;}
    bool getFloat(float& out)const{if(!isValid()||type!=AutomationArgumentType::FLOAT)return false;out=floatValue;return true;}
    bool getPercentage(uint8_t& out)const{if(!isValid()||type!=AutomationArgumentType::PERCENTAGE)return false;out=percentageValue;return true;}
    bool getDuration(uint32_t& out)const{if(!isValid()||type!=AutomationArgumentType::DURATION_MS)return false;out=durationMs;return true;}
    bool getIdentifier(uint32_t& out)const{if(!isValid()||type!=AutomationArgumentType::IDENTIFIER)return false;out=identifierValue;return true;}
    bool getEnum(int32_t& out)const{if(!isValid()||type!=AutomationArgumentType::ENUM_VALUE)return false;out=enumValue;return true;}
};

#endif
