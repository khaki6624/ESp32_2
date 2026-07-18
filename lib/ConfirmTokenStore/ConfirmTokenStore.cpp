#include <ConfirmTokenStore.h>

namespace
{
ConfirmTokenCheckResult validateToken(
    const ConfirmTokenRecord& token,
    CommandId commandId,
    RequestId requestId,
    uint32_t nowMs
)
{
    if (!token.isValid()) return ConfirmTokenCheckResult::POLICY_ERROR;
    if (token.consumed) return ConfirmTokenCheckResult::TOKEN_ALREADY_USED;
    if (token.commandId != commandId) return ConfirmTokenCheckResult::TOKEN_COMMAND_MISMATCH;
    if (token.requestId != requestId) return ConfirmTokenCheckResult::TOKEN_REQUEST_MISMATCH;
    if (token.isExpired(nowMs)) return ConfirmTokenCheckResult::EXPIRED_TOKEN;
    return ConfirmTokenCheckResult::VALID;
}
}

ConfirmTokenStore::ConfirmTokenStore() : tokens_{}, count_(0U) {}

void ConfirmTokenStore::clear()
{
    for (size_t index = 0U; index < count_; ++index) tokens_[index] = ConfirmTokenRecord{};
    count_ = 0U;
}

ConfirmTokenStoreResult ConfirmTokenStore::add(const ConfirmTokenRecord& token)
{
    if (!token.isValid()) return ConfirmTokenStoreResult::INVALID_TOKEN;
    if (findByValue(token.value) != nullptr) return ConfirmTokenStoreResult::DUPLICATE_VALUE;
    if (count_ >= CONFIRM_TOKEN_CAPACITY) return ConfirmTokenStoreResult::STORE_FULL;
    tokens_[count_++] = token;
    return ConfirmTokenStoreResult::SUCCESS;
}

const ConfirmTokenRecord* ConfirmTokenStore::findByValue(ConfirmToken value) const
{
    if (value == INVALID_CONFIRM_TOKEN) return nullptr;
    for (size_t index = 0U; index < count_; ++index)
        if (tokens_[index].value == value) return &tokens_[index];
    return nullptr;
}

ConfirmTokenRecord* ConfirmTokenStore::findMutableByValue(ConfirmToken value)
{
    if (value == INVALID_CONFIRM_TOKEN) return nullptr;
    for (size_t index = 0U; index < count_; ++index)
        if (tokens_[index].value == value) return &tokens_[index];
    return nullptr;
}

ConfirmTokenStoreResult ConfirmTokenStore::remove(ConfirmToken value)
{
    for (size_t index = 0U; index < count_; ++index)
    {
        if (tokens_[index].value != value) continue;
        for (size_t shift = index + 1U; shift < count_; ++shift) tokens_[shift - 1U] = tokens_[shift];
        --count_;
        tokens_[count_] = ConfirmTokenRecord{};
        return ConfirmTokenStoreResult::SUCCESS;
    }
    return ConfirmTokenStoreResult::TOKEN_NOT_FOUND;
}

ConfirmTokenCheckResult ConfirmTokenStore::check(
    ConfirmToken value, CommandId commandId, RequestId requestId, uint32_t nowMs
) const
{
    const ConfirmTokenRecord* token = findByValue(value);
    if (token == nullptr) return ConfirmTokenCheckResult::INVALID_TOKEN;
    return validateToken(*token, commandId, requestId, nowMs);
}

ConfirmTokenCheckResult ConfirmTokenStore::consume(
    ConfirmToken value, CommandId commandId, RequestId requestId, uint32_t nowMs
)
{
    ConfirmTokenRecord* token = findMutableByValue(value);
    if (token == nullptr) return ConfirmTokenCheckResult::INVALID_TOKEN;
    const ConfirmTokenCheckResult result = validateToken(*token, commandId, requestId, nowMs);
    if (result != ConfirmTokenCheckResult::VALID) return result;
    token->consumed = true;
    return ConfirmTokenCheckResult::VALID;
}

size_t ConfirmTokenStore::size() const { return count_; }
size_t ConfirmTokenStore::capacity() const { return CONFIRM_TOKEN_CAPACITY; }
