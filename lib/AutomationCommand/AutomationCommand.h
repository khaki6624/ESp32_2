#ifndef AUTOMATION_COMMAND_H
#define AUTOMATION_COMMAND_H

#include <AutomationCommandCommon.h>

struct AutomationCommand
{
    AutomationCommandArgument arguments[AUTOMATION_COMMAND_MAX_ARGUMENTS];
    uint32_t durationMs;
    uint16_t domainIndex;
    CommandOperation operation;
    CommandDomain domain;
    uint8_t argumentCount;
    bool hasDomainIndex;
    bool hasDurationValue;

    AutomationCommand() : arguments{},durationMs(0),domainIndex(0),operation(CommandOperation::NONE),
        domain(CommandDomain::NONE),argumentCount(0),hasDomainIndex(false),hasDurationValue(false) {}

    bool isValid() const { return isAutomationCommandValid(validate()); }
    AutomationCommandValidationResult validate() const
    {
        if(domain==CommandDomain::NONE||!isValidCommandDomain(domain))return AutomationCommandValidationResult::INVALID_DOMAIN;
        if((hasDomainIndex&&domainIndex==0U)||(!hasDomainIndex&&domainIndex!=0U))return AutomationCommandValidationResult::INVALID_DOMAIN_INDEX;
        if(operation==CommandOperation::NONE||!isValidCommandOperation(operation))return AutomationCommandValidationResult::INVALID_OPERATION;
        if(isDangerous(operation))return AutomationCommandValidationResult::DANGEROUS_OPERATION_NOT_PERSISTABLE;
        if(!isSupportedMapping(domain,operation))return AutomationCommandValidationResult::DOMAIN_OPERATION_MISMATCH;
        if(argumentCount>AUTOMATION_COMMAND_MAX_ARGUMENTS)return AutomationCommandValidationResult::TOO_MANY_ARGUMENTS;
        for(size_t i=0;i<argumentCount;++i)if(!arguments[i].isValid())return AutomationCommandValidationResult::INVALID_ARGUMENT;
        if(argumentCount!=0U)return AutomationCommandValidationResult::INVALID_ARGUMENT;
        if(hasDurationValue&&domain!=CommandDomain::OUT)return AutomationCommandValidationResult::INVALID_DURATION;
        if(operation==CommandOperation::PULSE&&(!hasDurationValue||durationMs==0U))return AutomationCommandValidationResult::INVALID_DURATION;
        return AutomationCommandValidationResult::VALID;
    }
    bool addArgument(const AutomationCommandArgument& value)
    {if(argumentCount>=AUTOMATION_COMMAND_MAX_ARGUMENTS||!value.isValid())return false;arguments[argumentCount++]=value;return true;}
    const AutomationCommandArgument* getArgument(size_t index)const{return index<argumentCount?&arguments[index]:nullptr;}
    bool setDuration(uint32_t valueMs){durationMs=valueMs;hasDurationValue=true;return true;}
    void clearDuration(){durationMs=0U;hasDurationValue=false;}
    void clear(){*this=AutomationCommand{};}

private:
    static bool isDangerous(CommandOperation value)
    {
        switch(value)
        {
            case CommandOperation::REBOOT: case CommandOperation::RESTORE:
            case CommandOperation::RESET: case CommandOperation::CLEAR:
            case CommandOperation::FACTORY: case CommandOperation::OTA:
            case CommandOperation::IMPORT_DATA: return true;
            default: return false;
        }
    }
    static bool isSupportedMapping(CommandDomain valueDomain,CommandOperation valueOperation)
    {
        switch(valueDomain)
        {
            case CommandDomain::OUT:return valueOperation==CommandOperation::ON||valueOperation==CommandOperation::OFF||valueOperation==CommandOperation::TOGGLE||valueOperation==CommandOperation::PULSE;
            case CommandDomain::IR:case CommandDomain::RF:case CommandDomain::SMS:return valueOperation==CommandOperation::SEND;
            case CommandDomain::NODE:return valueOperation==CommandOperation::PING||valueOperation==CommandOperation::SYNC||valueOperation==CommandOperation::DISCOVER;
            case CommandDomain::SCN:return valueOperation==CommandOperation::RUN||valueOperation==CommandOperation::STOP;
            case CommandDomain::CALL:return valueOperation==CommandOperation::START||valueOperation==CommandOperation::STOP;
            default:return false;
        }
    }
};

// AutomationCommand مستقیماً اجرا نمی‌شود. Factory آینده هنگام اجرا یک Command
// Runtime تازه می‌سازد و شناسه‌ها، Context، Risk، Priority، Timestamp و Source را
// از Context جاری تولید می‌کند. Authorization، Safety و Confirm نیز دوباره در
// Pipeline بررسی می‌شوند؛ این Patch هیچ Factory یا مسیر اجرایی ایجاد نمی‌کند.

#endif
