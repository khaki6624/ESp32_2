#ifndef COMPOSITE_COMMAND_EXECUTION_GATE_H
#define COMPOSITE_COMMAND_EXECUTION_GATE_H

#include <AuthorizationPolicy.h>
#include <CommandExecutionCommitHook.h>
#include <CommandExecutionGate.h>
#include <ConfirmTokenProvider.h>
#include <SafetyPolicy.h>
#include <SystemModeProvider.h>

class CompositeCommandExecutionGate final
    : public CommandExecutionGate,
      public CommandExecutionCommitHook
{
public:
    CompositeCommandExecutionGate(
        const SystemModeProvider& systemModeProvider,
        const AuthorizationPolicy& authorizationPolicy,
        const SafetyPolicy& safetyPolicy,
        ConfirmTokenProvider& confirmTokenProvider
    );

    CommandExecutionGateResult check(
        const Command& command, uint32_t nowMs
    ) const override;

    // این Hook فقط پس از Dispatch موفق توسط لایه یکپارچه‌ساز آینده فراخوانی می‌شود.
    CommandExecutionGateResult commit(
        const Command& command, uint32_t nowMs
    ) override;

private:
    const SystemModeProvider& systemModeProvider_;
    const AuthorizationPolicy& authorizationPolicy_;
    const SafetyPolicy& safetyPolicy_;
    ConfirmTokenProvider& confirmTokenProvider_;
};

#endif
