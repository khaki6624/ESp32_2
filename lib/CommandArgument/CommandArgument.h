#ifndef COMMAND_ARGUMENT_H
#define COMMAND_ARGUMENT_H
#include <math.h>
#include <CommandCommon.h>

struct CommandArgument
{
    CommandArgumentType type;
    union
    {
        bool booleanValue; int32_t integerValue; float floatValue; uint8_t percentageValue;
        uint32_t durationMs; uint32_t identifierValue; ConfirmToken tokenValue;
    };
    char textValue[COMMAND_TEXT_ARGUMENT_MAX_LENGTH];
    bool valid;

    CommandArgument() : type(CommandArgumentType::NONE),integerValue(0),textValue{},valid(false) {}
    void clear()
    {
        // Assignment از Object پیش‌فرض، Union و Buffer متن را نیز به‌طور کامل Reset می‌کند.
        *this=CommandArgument{};
    }
    void invalidate() { clear(); }
    static CommandArgument makeBoolean(bool v) { CommandArgument r; r.type=CommandArgumentType::BOOLEAN;r.booleanValue=v;r.valid=true;return r; }
    static CommandArgument makeInteger(int32_t v) { CommandArgument r;r.type=CommandArgumentType::INTEGER;r.integerValue=v;r.valid=true;return r; }
    static CommandArgument makeFloat(float v) { CommandArgument r;if(isfinite(v)){r.type=CommandArgumentType::FLOAT;r.floatValue=v;r.valid=true;}return r; }
    static CommandArgument makePercentage(int32_t v) { CommandArgument r;if(v>=0&&v<=100){r.type=CommandArgumentType::PERCENTAGE;r.percentageValue=static_cast<uint8_t>(v);r.valid=true;}return r; }
    static CommandArgument makeDuration(uint32_t v) { CommandArgument r;r.type=CommandArgumentType::DURATION_MS;r.durationMs=v;r.valid=true;return r; }
    static CommandArgument makeIdentifier(uint32_t v) { CommandArgument r;r.type=CommandArgumentType::IDENTIFIER;r.identifierValue=v;r.valid=true;return r; }
    static CommandArgument makeToken(ConfirmToken v) { CommandArgument r;if(v!=INVALID_CONFIRM_TOKEN){r.type=CommandArgumentType::TOKEN;r.tokenValue=v;r.valid=true;}return r; }
    static CommandArgument makeText(CommandArgumentType textType,const char* value)
    {
        CommandArgument r;
        if(textType!=CommandArgumentType::DATE&&textType!=CommandArgumentType::TIME&&
           textType!=CommandArgumentType::TEXT&&textType!=CommandArgumentType::COMMAND_TEXT) return r;
        if(!CommandText::set(r.textValue,sizeof(r.textValue),value)) return r;
        r.type=textType;r.valid=true;return r;
    }
    bool isValid() const
    {
        if(!valid||type==CommandArgumentType::NONE||!isValidCommandArgumentType(type)) return false;
        if(type==CommandArgumentType::FLOAT&&!isfinite(floatValue)) return false;
        if(type==CommandArgumentType::PERCENTAGE&&percentageValue>100U) return false;
        if(type==CommandArgumentType::TOKEN&&tokenValue==INVALID_CONFIRM_TOKEN) return false;
        if(type==CommandArgumentType::DATE||type==CommandArgumentType::TIME||
           type==CommandArgumentType::TEXT||type==CommandArgumentType::COMMAND_TEXT)
            return CommandText::isCanonical(textValue,sizeof(textValue));
        return true;
    }
    bool getBoolean(bool& out) const { if(!isValid()||type!=CommandArgumentType::BOOLEAN)return false;out=booleanValue;return true; }
    bool getInteger(int32_t& out) const { if(!isValid()||type!=CommandArgumentType::INTEGER)return false;out=integerValue;return true; }
    bool getFloat(float& out) const { if(!isValid()||type!=CommandArgumentType::FLOAT)return false;out=floatValue;return true; }
    bool getPercentage(uint8_t& out) const { if(!isValid()||type!=CommandArgumentType::PERCENTAGE)return false;out=percentageValue;return true; }
    bool getDuration(uint32_t& out) const { if(!isValid()||type!=CommandArgumentType::DURATION_MS)return false;out=durationMs;return true; }
    bool getIdentifier(uint32_t& out) const { if(!isValid()||type!=CommandArgumentType::IDENTIFIER)return false;out=identifierValue;return true; }
    bool getToken(ConfirmToken& out) const { if(!isValid()||type!=CommandArgumentType::TOKEN)return false;out=tokenValue;return true; }
    bool getText(const char*& out,CommandArgumentType expectedType) const
    {
        if(!isValid()||type!=expectedType||(type!=CommandArgumentType::DATE&&type!=CommandArgumentType::TIME&&
           type!=CommandArgumentType::TEXT&&type!=CommandArgumentType::COMMAND_TEXT)) return false;
        out=textValue;return true;
    }
};
#endif
