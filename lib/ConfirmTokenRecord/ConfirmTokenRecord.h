#ifndef CONFIRM_TOKEN_RECORD_H
#define CONFIRM_TOKEN_RECORD_H

#include <CommandCommon.h>
#include <ConfirmTokenRuntimeCommon.h>

struct ConfirmTokenRecord
{
    ConfirmTokenId id;
    ConfirmToken value;
    CommandId commandId;
    RequestId requestId;
    uint32_t issuedAtMs;
    uint32_t ttlMs;
    bool consumed;

    ConfirmTokenRecord() : id(INVALID_CONFIRM_TOKEN_ID), value(INVALID_CONFIRM_TOKEN),
        commandId(INVALID_COMMAND_ID), requestId(INVALID_REQUEST_ID), issuedAtMs(0U),
        ttlMs(0U), consumed(false) {}

    bool isValid() const
    {
        return id != INVALID_CONFIRM_TOKEN_ID && value != INVALID_CONFIRM_TOKEN &&
            commandId != INVALID_COMMAND_ID && requestId != INVALID_REQUEST_ID &&
            ttlMs != 0U && ttlMs <= CONFIRM_TOKEN_MAX_TTL_MS;
    }

    bool isExpired(uint32_t nowMs) const
    {
        return static_cast<uint32_t>(nowMs - issuedAtMs) >= ttlMs;
    }
};

#endif
