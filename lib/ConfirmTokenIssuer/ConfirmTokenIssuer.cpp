#include <ConfirmTokenIssuer.h>

ConfirmTokenIssuer::ConfirmTokenIssuer(
    ConfirmTokenStore& store, ConfirmTokenGenerator& generator, uint32_t tokenTtlMs
) : store_(store), generator_(generator), tokenTtlMs_(tokenTtlMs), nextId_(1U) {}

ConfirmTokenIssueResult ConfirmTokenIssuer::issue(
    const Command& command, uint32_t nowMs, ConfirmToken& output
)
{
    if (!command.isValid() || command.context.risk != CommandRisk::DANGEROUS)
        return ConfirmTokenIssueResult::INVALID_COMMAND;
    if (tokenTtlMs_ == 0U || tokenTtlMs_ > CONFIRM_TOKEN_MAX_TTL_MS)
        return ConfirmTokenIssueResult::INVALID_TTL;
    if (store_.size() >= store_.capacity()) return ConfirmTokenIssueResult::STORE_FULL;
    ConfirmToken value = INVALID_CONFIRM_TOKEN;
    if (!generator_.generate(value)) return ConfirmTokenIssueResult::GENERATION_FAILED;
    if (value == INVALID_CONFIRM_TOKEN) return ConfirmTokenIssueResult::INVALID_GENERATED_VALUE;
    if (store_.findByValue(value) != nullptr) return ConfirmTokenIssueResult::DUPLICATE_VALUE;
    ConfirmTokenRecord token;
    token.id = nextId_;
    token.value = value;
    token.commandId = command.context.commandId;
    token.requestId = command.context.request.requestId;
    token.issuedAtMs = nowMs;
    token.ttlMs = tokenTtlMs_;
    const ConfirmTokenStoreResult stored = store_.add(token);
    if (stored != ConfirmTokenStoreResult::SUCCESS) return ConfirmTokenIssueResult::STORE_ERROR;
    output = value;
    ++nextId_;
    if (nextId_ == INVALID_CONFIRM_TOKEN_ID) nextId_ = 1U;
    return ConfirmTokenIssueResult::SUCCESS;
}

uint32_t ConfirmTokenIssuer::tokenTtlMs() const { return tokenTtlMs_; }
