#ifndef COMMAND_H
#define COMMAND_H
#include <CommandArgument.h>
#include <CommandContext.h>
#include <CommandPath.h>

struct Command
{
    CommandContext context;
    CommandDomain domain;
    uint16_t domainIndex;
    bool hasDomainIndex;
    CommandPath path;
    CommandQueryType queryType;
    CommandOperation operation;
    CommandArgument arguments[COMMAND_MAX_ARGUMENTS];
    uint8_t argumentCount;
    uint32_t durationMs;
    bool hasDurationValue;
    char originalText[COMMAND_ORIGINAL_TEXT_MAX_LENGTH];

    Command() : context{},domain(CommandDomain::NONE),domainIndex(0),hasDomainIndex(false),
        path{},queryType(CommandQueryType::NONE),operation(CommandOperation::NONE),arguments{},
        argumentCount(0),durationMs(0),hasDurationValue(false),originalText{} {}
    bool isValid() const
    {
        if(!context.isValid()||domain==CommandDomain::NONE||!isValidCommandDomain(domain)||
           !path.isValid()||!isValidCommandQueryType(queryType)||!isValidCommandOperation(operation)||
           argumentCount>COMMAND_MAX_ARGUMENTS||!CommandText::isTerminated(originalText,sizeof(originalText))) return false;
        if((hasDomainIndex&&domainIndex==0)||(!hasDomainIndex&&domainIndex!=0)) return false;
        for(size_t i=0;i<argumentCount;++i) if(!arguments[i].isValid()) return false;
        if(queryType!=CommandQueryType::NONE)
            return operation==CommandOperation::NONE&&argumentCount==0&&!hasDurationValue;
        return operation!=CommandOperation::NONE;
    }
    bool isQuery() const { return queryType!=CommandQueryType::NONE; }
    bool isAction() const { return queryType==CommandQueryType::NONE&&operation!=CommandOperation::NONE; }
    bool hasDuration() const { return hasDurationValue; }
    bool addArgument(const CommandArgument& value)
    { if(argumentCount>=COMMAND_MAX_ARGUMENTS||!value.isValid())return false;arguments[argumentCount++]=value;return true; }
    const CommandArgument* getArgument(size_t index) const { return index<argumentCount?&arguments[index]:nullptr; }
    bool setDuration(uint32_t valueMs) { durationMs=valueMs;hasDurationValue=true;return true; }
    void clearDuration() { durationMs=0;hasDurationValue=false; }
    bool setOriginalText(const char* value) { return CommandText::set(originalText,sizeof(originalText),value); }
};
#endif
