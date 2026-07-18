#ifndef CONFIRM_TOKEN_ISSUER_H
#define CONFIRM_TOKEN_ISSUER_H

#include <Command.h>
#include <ConfirmTokenGenerator.h>
#include <ConfirmTokenStore.h>

class ConfirmTokenIssuer
{
public:
    ConfirmTokenIssuer(ConfirmTokenStore& store, ConfirmTokenGenerator& generator, uint32_t tokenTtlMs);
    ConfirmTokenIssueResult issue(const Command& command, uint32_t nowMs, ConfirmToken& output);
    uint32_t tokenTtlMs() const;
private:
    ConfirmTokenStore& store_;
    ConfirmTokenGenerator& generator_;
    uint32_t tokenTtlMs_;
    ConfirmTokenId nextId_;
};

#endif
