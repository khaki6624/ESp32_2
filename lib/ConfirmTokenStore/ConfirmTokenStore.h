#ifndef CONFIRM_TOKEN_STORE_H
#define CONFIRM_TOKEN_STORE_H

#include <stddef.h>

#include <ConfirmTokenRecord.h>
#include <SecurityCommon.h>

constexpr size_t CONFIRM_TOKEN_CAPACITY = 8U;

class ConfirmTokenStore
{
public:
    ConfirmTokenStore();
    void clear();
    ConfirmTokenStoreResult add(const ConfirmTokenRecord& token);
    const ConfirmTokenRecord* findByValue(ConfirmToken value) const;
    ConfirmTokenStoreResult remove(ConfirmToken value);
    ConfirmTokenCheckResult check(
        ConfirmToken value, CommandId commandId, RequestId requestId, uint32_t nowMs
    ) const;
    ConfirmTokenCheckResult consume(
        ConfirmToken value, CommandId commandId, RequestId requestId, uint32_t nowMs
    );
    size_t size() const;
    size_t capacity() const;

private:
    ConfirmTokenRecord tokens_[CONFIRM_TOKEN_CAPACITY];
    size_t count_;
    ConfirmTokenRecord* findMutableByValue(ConfirmToken value);
};

#endif
