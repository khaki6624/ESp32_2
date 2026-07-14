#include "AutomationCommandFactory.h"

AutomationCommandFactory::AutomationCommandFactory(){}

AutomationCommandFactoryResult AutomationCommandFactory::create(const AutomationCommand& source,
    const RequestContext& request,CommandId commandId,uint32_t createdTimestampMs,Command& output) const
{
    if(!source.isValid())return AutomationCommandFactoryResult::INVALID_AUTOMATION_COMMAND;
    if(!request.isValid())return request.requestId==INVALID_REQUEST_ID?
        AutomationCommandFactoryResult::INVALID_REQUEST_ID:AutomationCommandFactoryResult::INVALID_REQUEST_CONTEXT;
    if(commandId==INVALID_COMMAND_ID)return AutomationCommandFactoryResult::INVALID_COMMAND_ID;

    Command temporary;
    temporary.context.commandId=commandId;
    temporary.context.request=request;
    if(!mapRisk(source,temporary.context.risk))return AutomationCommandFactoryResult::UNSUPPORTED_OPERATION;
    temporary.context.priority=CommandPriority::NORMAL;
    temporary.context.createdTimestampMs=createdTimestampMs;
    temporary.context.timeoutMs=0U;
    temporary.context.confirmToken=INVALID_CONFIRM_TOKEN;
    temporary.context.confirmationRequired=false;
    temporary.context.retryCount=0U;
    temporary.context.maxRetries=0U;
    temporary.domain=source.domain;
    temporary.domainIndex=source.domainIndex;
    temporary.hasDomainIndex=source.hasDomainIndex;
    temporary.queryType=CommandQueryType::NONE;
    temporary.operation=source.operation;

    for(size_t index=0;index<source.argumentCount;++index)
    {
        CommandArgument argument;
        if(!convertArgument(source.arguments[index],argument)||!temporary.addArgument(argument))
            return AutomationCommandFactoryResult::ARGUMENT_CONVERSION_FAILED;
    }
    if(source.hasDurationValue)temporary.setDuration(source.durationMs);else temporary.clearDuration();
    if(!temporary.isValid())return AutomationCommandFactoryResult::OUTPUT_COMMAND_INVALID;
    output=temporary;
    return AutomationCommandFactoryResult::SUCCESS;
}

bool AutomationCommandFactory::mapRisk(const AutomationCommand& source,CommandRisk& output)
{
    if(source.domain==CommandDomain::NODE&&(source.operation==CommandOperation::PING||
       source.operation==CommandOperation::SYNC||source.operation==CommandOperation::DISCOVER))
    {output=CommandRisk::SAFE;return true;}
    switch(source.domain)
    {
        case CommandDomain::OUT: case CommandDomain::IR: case CommandDomain::RF:
        case CommandDomain::SCN: case CommandDomain::SMS: case CommandDomain::CALL:
            output=CommandRisk::ACTION;return true;
        default:return false;
    }
}

bool AutomationCommandFactory::convertArgument(const AutomationCommandArgument& source,CommandArgument& output)
{
    if(!source.isValid())return false;
    switch(source.type)
    {
        case AutomationArgumentType::BOOLEAN:output=CommandArgument::makeBoolean(source.booleanValue);break;
        case AutomationArgumentType::INTEGER:output=CommandArgument::makeInteger(source.integerValue);break;
        case AutomationArgumentType::FLOAT:output=CommandArgument::makeFloat(source.floatValue);break;
        case AutomationArgumentType::PERCENTAGE:output=CommandArgument::makePercentage(source.percentageValue);break;
        case AutomationArgumentType::DURATION_MS:output=CommandArgument::makeDuration(source.durationMs);break;
        case AutomationArgumentType::IDENTIFIER:output=CommandArgument::makeIdentifier(source.identifierValue);break;
        // CommandArgument نوع Enum مستقل ندارد؛ مقدار Normalized با INTEGER حفظ می‌شود.
        case AutomationArgumentType::ENUM_VALUE:output=CommandArgument::makeInteger(source.enumValue);break;
        case AutomationArgumentType::NONE:default:return false;
    }
    return output.isValid();
}
