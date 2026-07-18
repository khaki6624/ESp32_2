#ifndef RUNTIME_CONFIRM_TOKEN_PROVIDER_H
#define RUNTIME_CONFIRM_TOKEN_PROVIDER_H

#include <ConfirmTokenProvider.h>
#include <ConfirmTokenStore.h>

class RuntimeConfirmTokenProvider final : public ConfirmTokenProvider
{
public:
    explicit RuntimeConfirmTokenProvider(ConfirmTokenStore& store);
    ConfirmTokenCheckResult check(const Command& command, uint32_t nowMs) const override;
    ConfirmTokenCheckResult consume(const Command& command, uint32_t nowMs) override;
private:
    ConfirmTokenStore& store_;
};

#endif
