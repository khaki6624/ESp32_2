#ifndef COMMAND_CONTEXT_H
#define COMMAND_CONTEXT_H
#include <RequestContext.h>

struct CommandContext
{
    CommandId commandId;
    RequestContext request;
    CommandRisk risk;
    CommandPriority priority;
    uint32_t createdTimestampMs;
    uint32_t timeoutMs;
    ConfirmToken confirmToken;
    bool confirmationRequired;
    uint8_t retryCount;
    uint8_t maxRetries;

    CommandContext() : commandId(INVALID_COMMAND_ID),request{},risk(CommandRisk::SAFE),
        priority(CommandPriority::NORMAL),createdTimestampMs(0),timeoutMs(0),
        confirmToken(INVALID_CONFIRM_TOKEN),confirmationRequired(false),retryCount(0),maxRetries(0) {}
    bool isValid() const
    { return commandId!=INVALID_COMMAND_ID && request.isValid() && isValidCommandRisk(risk) &&
             isValidCommandPriority(priority) && retryCount<=maxRetries; }
    bool requiresConfirmation() const { return confirmationRequired; }
    bool hasConfirmToken() const { return confirmToken!=INVALID_CONFIRM_TOKEN; }
};
#endif
