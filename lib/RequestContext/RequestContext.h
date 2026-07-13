#ifndef REQUEST_CONTEXT_H
#define REQUEST_CONTEXT_H
#include <CommandCommon.h>

struct RequestContext
{
    RequestId requestId;
    CommandSource source;
    UserId userId;
    SessionId sessionId;
    uint32_t receivedTimestampMs;
    char sourceName[COMMAND_SOURCE_NAME_MAX_LENGTH];

    RequestContext() : requestId(INVALID_REQUEST_ID),source(CommandSource::NONE),
        userId(INVALID_USER_ID),sessionId(INVALID_SESSION_ID),receivedTimestampMs(0),sourceName{} {}
    bool isValid() const
    {
        return requestId!=INVALID_REQUEST_ID && source!=CommandSource::NONE &&
               isValidCommandSource(source) && CommandText::isCanonical(sourceName,sizeof(sourceName));
    }
    bool setSourceName(const char* value) { return CommandText::set(sourceName,sizeof(sourceName),value); }
};
#endif
