#include <CompositeCommandExecutionGate.h>
#include <CommandExecutor.h>
#include <Arduino.h>
#include <unity.h>

namespace
{
Command makeCommand(CommandRisk risk=CommandRisk::SAFE)
{
    Command c;c.context.commandId=10U;c.context.request.requestId=20U;
    c.context.request.source=static_cast<CommandSource>(5U);c.context.risk=risk;
    c.context.createdTimestampMs=1U;c.domain=CommandDomain::OUT;c.domainIndex=1U;
    c.hasDomainIndex=true;c.operation=CommandOperation::ON;return c;
}
class ModeFake final:public SystemModeProvider{public:SystemMode mode=SystemMode::NORMAL;SystemModeCheckResult result=SystemModeCheckResult::ALLOWED;mutable uint8_t calls=0;SystemMode getMode()const override{return mode;}SystemModeCheckResult check(const Command&,uint32_t)const override{++calls;return result;}};
class AuthFake final:public AuthorizationPolicy{public:AuthorizationCheckResult result=AuthorizationCheckResult::ALLOWED;mutable uint8_t calls=0;AuthorizationCheckResult check(const Command&,uint32_t)const override{++calls;return result;}};
class SafetyFake final:public SafetyPolicy{public:SafetyCheckResult result=SafetyCheckResult::ALLOWED;mutable uint8_t calls=0;SafetyCheckResult check(const Command&,uint32_t)const override{++calls;return result;}};
class TokenFake final:public ConfirmTokenProvider{public:ConfirmTokenCheckResult result=ConfirmTokenCheckResult::NOT_REQUIRED;ConfirmTokenCheckResult consumeResult=ConfirmTokenCheckResult::VALID;mutable uint8_t checks=0;uint8_t consumes=0;ConfirmTokenCheckResult check(const Command&,uint32_t)const override{++checks;return result;}ConfirmTokenCheckResult consume(const Command&,uint32_t)override{++consumes;return consumeResult;}};
class HandlerFake final:public CommandHandler{public:mutable uint8_t calls=0;bool supports(const Command&)const override{return true;}CommandDispatchResult handle(const Command& c,CommandResult& out)const override{++calls;out.commandId=c.context.commandId;out.requestId=c.context.request.requestId;out.transitionTo(ExecutionStatus::VALIDATING,1U);out.transitionTo(ExecutionStatus::ACCEPTED,2U);out.transitionTo(ExecutionStatus::EXECUTING,3U);out.transitionTo(ExecutionStatus::SUCCESS,4U);return CommandDispatchResult::SUCCESS;}};
struct Fixture{ModeFake mode;AuthFake auth;SafetyFake safety;TokenFake token;CompositeCommandExecutionGate gate;Fixture():gate(mode,auth,safety,token){}};
void drive(CommandExecutor& e){for(uint32_t i=1U;i<=5U;++i)e.update(i);}
}
void setUp(){} void tearDown(){}

void test_invalid_command_and_risk_are_rejected()
{Fixture f;Command c;TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::INVALID_COMMAND,(uint8_t)f.gate.check(c,1U));c=makeCommand();c.context.risk=static_cast<CommandRisk>(99U);TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::INVALID_COMMAND,(uint8_t)f.gate.check(c,1U));TEST_ASSERT_EQUAL_UINT8(0U,f.mode.calls);}

void test_safe_command_runs_in_exact_order()
{Fixture f;Command c=makeCommand();TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::ALLOWED,(uint8_t)f.gate.check(c,7U));TEST_ASSERT_EQUAL_UINT8(1U,f.mode.calls);TEST_ASSERT_EQUAL_UINT8(1U,f.auth.calls);TEST_ASSERT_EQUAL_UINT8(1U,f.token.checks);TEST_ASSERT_EQUAL_UINT8(1U,f.safety.calls);TEST_ASSERT_EQUAL_UINT8(0U,f.token.consumes);}

void test_failures_short_circuit_without_consuming()
{Fixture f;Command c=makeCommand();f.mode.result=SystemModeCheckResult::SYSTEM_LOCKED;TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::SYSTEM_MODE_REJECTED,(uint8_t)f.gate.check(c,1U));TEST_ASSERT_EQUAL_UINT8(0U,f.auth.calls);f.mode.result=SystemModeCheckResult::ALLOWED;f.auth.result=AuthorizationCheckResult::USER_DISABLED;TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::UNAUTHORIZED,(uint8_t)f.gate.check(c,1U));TEST_ASSERT_EQUAL_UINT8(0U,f.token.checks);f.auth.result=AuthorizationCheckResult::ALLOWED;f.token.result=ConfirmTokenCheckResult::REQUIRED;TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::CONFIRMATION_REQUIRED,(uint8_t)f.gate.check(c,1U));TEST_ASSERT_EQUAL_UINT8(0U,f.safety.calls);TEST_ASSERT_EQUAL_UINT8(0U,f.token.consumes);}

void test_mode_authorization_and_safety_mappings()
{Fixture f;Command c=makeCommand();const SystemModeCheckResult modes[]={SystemModeCheckResult::SYSTEM_LOCKED,SystemModeCheckResult::MAINTENANCE_REJECTED,SystemModeCheckResult::SYSTEM_UPDATING,SystemModeCheckResult::EMERGENCY_LOCK_ACTIVE};for(auto v:modes){f.mode.result=v;TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::SYSTEM_MODE_REJECTED,(uint8_t)f.gate.check(c,1U));}f.mode.result=SystemModeCheckResult::ALLOWED;const AuthorizationCheckResult auths[]={AuthorizationCheckResult::USER_NOT_FOUND,AuthorizationCheckResult::USER_DISABLED,AuthorizationCheckResult::PERMISSION_DENIED};for(auto v:auths){f.auth.result=v;TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::UNAUTHORIZED,(uint8_t)f.gate.check(c,1U));}f.auth.result=AuthorizationCheckResult::POLICY_ERROR;TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::POLICY_ERROR,(uint8_t)f.gate.check(c,1U));f.auth.result=AuthorizationCheckResult::ALLOWED;const SafetyCheckResult safeties[]={SafetyCheckResult::INTERLOCK_ACTIVE,SafetyCheckResult::DEVICE_UNSAFE,SafetyCheckResult::NODE_UNSAFE,SafetyCheckResult::CRITICAL_FAULT};for(auto v:safeties){f.safety.result=v;TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::SAFETY_REJECTED,(uint8_t)f.gate.check(c,1U));}f.safety.result=SafetyCheckResult::INVALID_COMMAND;TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::INVALID_COMMAND,(uint8_t)f.gate.check(c,1U));}

void test_confirmation_risk_policy_and_failures()
{Fixture f;Command c=makeCommand();for(uint8_t r=0U;r<3U;++r){c.context.risk=static_cast<CommandRisk>(r);f.token.result=ConfirmTokenCheckResult::NOT_REQUIRED;TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::ALLOWED,(uint8_t)f.gate.check(c,1U));}c.context.risk=CommandRisk::DANGEROUS;f.token.result=ConfirmTokenCheckResult::NOT_REQUIRED;TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::POLICY_ERROR,(uint8_t)f.gate.check(c,1U));f.token.result=ConfirmTokenCheckResult::VALID;TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::ALLOWED,(uint8_t)f.gate.check(c,1U));const ConfirmTokenCheckResult bad[]={ConfirmTokenCheckResult::INVALID_TOKEN,ConfirmTokenCheckResult::EXPIRED_TOKEN,ConfirmTokenCheckResult::TOKEN_COMMAND_MISMATCH,ConfirmTokenCheckResult::TOKEN_REQUEST_MISMATCH,ConfirmTokenCheckResult::TOKEN_ALREADY_USED};for(auto v:bad){f.token.result=v;TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::INVALID_CONFIRM_TOKEN,(uint8_t)f.gate.check(c,1U));}}

void test_check_does_not_mutate_command_or_token()
{Fixture f;Command c=makeCommand(CommandRisk::DANGEROUS);c.context.confirmToken=123U;f.token.result=ConfirmTokenCheckResult::VALID;const CommandId id=c.context.commandId;const ConfirmToken token=c.context.confirmToken;f.gate.check(c,9U);TEST_ASSERT_EQUAL_UINT32(id,c.context.commandId);TEST_ASSERT_EQUAL_UINT32(token,c.context.confirmToken);TEST_ASSERT_EQUAL_UINT8(0U,f.token.consumes);}

void test_commit_is_explicit_and_consumes_once_per_call()
{Fixture f;Command c=makeCommand(CommandRisk::DANGEROUS);f.token.result=ConfirmTokenCheckResult::VALID;TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::ALLOWED,(uint8_t)f.gate.check(c,1U));TEST_ASSERT_EQUAL_UINT8(0U,f.token.consumes);TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutionGateResult::ALLOWED,(uint8_t)f.gate.commit(c,2U));TEST_ASSERT_EQUAL_UINT8(1U,f.token.consumes);}

void test_executor_integration_blocks_and_allows_dispatch()
{SceneExecutionQueue q;CommandValidator validator;Fixture f;HandlerFake handler;CommandDispatcher dispatcher;dispatcher.registerHandler(handler);CommandExecutor executor(q,validator,f.gate,dispatcher);f.auth.result=AuthorizationCheckResult::UNAUTHORIZED;q.enqueue(makeCommand());drive(executor);TEST_ASSERT_EQUAL_UINT8(0U,handler.calls);TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandErrorCode::UNAUTHORIZED,(uint8_t)executor.getLastCommandResult()->errorCode);executor.reset();f.auth.result=AuthorizationCheckResult::ALLOWED;q.enqueue(makeCommand());drive(executor);TEST_ASSERT_EQUAL_UINT8(1U,handler.calls);TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutorState::COMPLETED,(uint8_t)executor.getState());}

void setup(){UNITY_BEGIN();RUN_TEST(test_invalid_command_and_risk_are_rejected);RUN_TEST(test_safe_command_runs_in_exact_order);RUN_TEST(test_failures_short_circuit_without_consuming);RUN_TEST(test_mode_authorization_and_safety_mappings);RUN_TEST(test_confirmation_risk_policy_and_failures);RUN_TEST(test_check_does_not_mutate_command_or_token);RUN_TEST(test_commit_is_explicit_and_consumes_once_per_call);RUN_TEST(test_executor_integration_blocks_and_allows_dispatch);UNITY_END();}void loop(){}
