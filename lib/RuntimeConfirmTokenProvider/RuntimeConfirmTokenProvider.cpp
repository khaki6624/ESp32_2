#include <RuntimeConfirmTokenProvider.h>

namespace
{
ConfirmTokenCheckResult validateRequest(const Command& command)
{
    if (!command.isValid() || !isValidCommandRisk(command.context.risk))
        return ConfirmTokenCheckResult::POLICY_ERROR;
    if (command.context.risk != CommandRisk::DANGEROUS)
        return ConfirmTokenCheckResult::NOT_REQUIRED;
    if (!command.context.hasConfirmToken()) return ConfirmTokenCheckResult::REQUIRED;
    return ConfirmTokenCheckResult::VALID;
}
}

RuntimeConfirmTokenProvider::RuntimeConfirmTokenProvider(ConfirmTokenStore& store) : store_(store) {}

ConfirmTokenCheckResult RuntimeConfirmTokenProvider::check(const Command& command, uint32_t nowMs) const
{
    const ConfirmTokenCheckResult request = validateRequest(command);
    if (request != ConfirmTokenCheckResult::VALID) return request;
    return store_.check(command.context.confirmToken, command.context.commandId,
                        command.context.request.requestId, nowMs);
}

ConfirmTokenCheckResult RuntimeConfirmTokenProvider::consume(const Command& command, uint32_t nowMs)
{
    const ConfirmTokenCheckResult request = validateRequest(command);
    if (request != ConfirmTokenCheckResult::VALID) return request;
    return store_.consume(command.context.confirmToken, command.context.commandId,
                          command.context.request.requestId, nowMs);
}
