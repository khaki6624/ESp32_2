#include <CompositeCommandExecutionGate.h>

namespace
{
    CommandExecutionGateResult mapSystemMode(SystemModeCheckResult result)
    {
        switch (result)
        {
            case SystemModeCheckResult::ALLOWED:
                return CommandExecutionGateResult::ALLOWED;
            case SystemModeCheckResult::SYSTEM_LOCKED:
            case SystemModeCheckResult::MAINTENANCE_REJECTED:
            case SystemModeCheckResult::SYSTEM_UPDATING:
            case SystemModeCheckResult::EMERGENCY_LOCK_ACTIVE:
                return CommandExecutionGateResult::SYSTEM_MODE_REJECTED;
            case SystemModeCheckResult::INVALID_MODE:
            case SystemModeCheckResult::POLICY_ERROR:
            default:
                return CommandExecutionGateResult::POLICY_ERROR;
        }
    }

    CommandExecutionGateResult mapAuthorization(AuthorizationCheckResult result)
    {
        switch (result)
        {
            case AuthorizationCheckResult::ALLOWED:
                return CommandExecutionGateResult::ALLOWED;
            case AuthorizationCheckResult::USER_NOT_FOUND:
            case AuthorizationCheckResult::USER_DISABLED:
            case AuthorizationCheckResult::UNAUTHORIZED:
            case AuthorizationCheckResult::PERMISSION_DENIED:
                return CommandExecutionGateResult::UNAUTHORIZED;
            case AuthorizationCheckResult::INVALID_REQUEST:
            case AuthorizationCheckResult::POLICY_ERROR:
            default:
                return CommandExecutionGateResult::POLICY_ERROR;
        }
    }

    CommandExecutionGateResult mapConfirmation(
        ConfirmTokenCheckResult result, CommandRisk risk
    )
    {
        if (result == ConfirmTokenCheckResult::VALID)
            return CommandExecutionGateResult::ALLOWED;
        if (result == ConfirmTokenCheckResult::REQUIRED)
            return CommandExecutionGateResult::CONFIRMATION_REQUIRED;
        if (result == ConfirmTokenCheckResult::NOT_REQUIRED)
            return risk == CommandRisk::DANGEROUS
                ? CommandExecutionGateResult::POLICY_ERROR
                : CommandExecutionGateResult::ALLOWED;
        if (result == ConfirmTokenCheckResult::POLICY_ERROR)
            return CommandExecutionGateResult::POLICY_ERROR;
        return CommandExecutionGateResult::INVALID_CONFIRM_TOKEN;
    }

    CommandExecutionGateResult mapSafety(SafetyCheckResult result)
    {
        switch (result)
        {
            case SafetyCheckResult::ALLOWED:
                return CommandExecutionGateResult::ALLOWED;
            case SafetyCheckResult::INTERLOCK_ACTIVE:
            case SafetyCheckResult::DEVICE_UNSAFE:
            case SafetyCheckResult::NODE_UNSAFE:
            case SafetyCheckResult::TIME_RESTRICTED:
            case SafetyCheckResult::CRITICAL_FAULT:
                return CommandExecutionGateResult::SAFETY_REJECTED;
            case SafetyCheckResult::INVALID_COMMAND:
                return CommandExecutionGateResult::INVALID_COMMAND;
            case SafetyCheckResult::POLICY_ERROR:
            default:
                return CommandExecutionGateResult::POLICY_ERROR;
        }
    }
}

CompositeCommandExecutionGate::CompositeCommandExecutionGate(
    const SystemModeProvider& systemModeProvider,
    const AuthorizationPolicy& authorizationPolicy,
    const SafetyPolicy& safetyPolicy,
    ConfirmTokenProvider& confirmTokenProvider
) :
    systemModeProvider_(systemModeProvider),
    authorizationPolicy_(authorizationPolicy),
    safetyPolicy_(safetyPolicy),
    confirmTokenProvider_(confirmTokenProvider)
{
}

CommandExecutionGateResult CompositeCommandExecutionGate::check(
    const Command& command, uint32_t nowMs
) const
{
    if (!command.isValid() || !isValidCommandRisk(command.context.risk))
        return CommandExecutionGateResult::INVALID_COMMAND;

    CommandExecutionGateResult result = mapSystemMode(
        systemModeProvider_.check(command, nowMs)
    );
    if (result != CommandExecutionGateResult::ALLOWED)
        return result;

    result = mapAuthorization(authorizationPolicy_.check(command, nowMs));
    if (result != CommandExecutionGateResult::ALLOWED)
        return result;

    result = mapConfirmation(
        confirmTokenProvider_.check(command, nowMs), command.context.risk
    );
    if (result != CommandExecutionGateResult::ALLOWED)
        return result;

    return mapSafety(safetyPolicy_.check(command, nowMs));
}

CommandExecutionGateResult CompositeCommandExecutionGate::commit(
    const Command& command, uint32_t nowMs
)
{
    if (!command.isValid() || !isValidCommandRisk(command.context.risk))
        return CommandExecutionGateResult::INVALID_COMMAND;
    return mapConfirmation(
        confirmTokenProvider_.consume(command, nowMs), command.context.risk
    );
}
