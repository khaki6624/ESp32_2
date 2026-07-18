#include <Arduino.h>
#include <CommandExecutor.h>
#include <CompositeCommandExecutionGate.h>
#include <unity.h>

namespace
{
uint8_t sequence = 0U;

Command makeCommand(CommandRisk risk = CommandRisk::SAFE)
{
    Command command;
    command.context.commandId = 10U;
    command.context.request.requestId = 20U;
    command.context.request.source = static_cast<CommandSource>(5U);
    command.context.createdTimestampMs = 30U;
    command.context.risk = risk;
    command.domain = CommandDomain::OUT;
    command.domainIndex = 1U;
    command.hasDomainIndex = true;
    command.operation = CommandOperation::ON;
    return command;
}

CommandResult successResult(const Command& command)
{
    CommandResult result;
    result.commandId = command.context.commandId;
    result.requestId = command.context.request.requestId;
    result.transitionTo(ExecutionStatus::VALIDATING, 1U);
    result.transitionTo(ExecutionStatus::ACCEPTED, 2U);
    result.transitionTo(ExecutionStatus::EXECUTING, 3U);
    result.transitionTo(ExecutionStatus::SUCCESS, 4U);
    return result;
}

class FakeGate final : public CommandExecutionGate
{
public:
    CommandExecutionGateResult result = CommandExecutionGateResult::ALLOWED;
    mutable uint8_t calls = 0U;
    mutable uint8_t order = 0U;
    mutable uint32_t now = 0U;
    CommandExecutionGateResult check(const Command&, uint32_t nowMs) const override
    {
        ++calls; order = ++sequence; now = nowMs; return result;
    }
};

class FakeCommitHook final : public CommandExecutionCommitHook
{
public:
    CommandExecutionGateResult result = CommandExecutionGateResult::ALLOWED;
    uint8_t calls = 0U;
    uint8_t order = 0U;
    uint32_t now = 0U;
    CommandExecutionGateResult commit(const Command&, uint32_t nowMs) override
    {
        ++calls; order = ++sequence; now = nowMs; return result;
    }
};

class FakeHandler final : public CommandHandler
{
public:
    CommandDispatchResult result = CommandDispatchResult::SUCCESS;
    mutable uint8_t calls = 0U;
    mutable uint8_t order = 0U;
    bool supports(const Command&) const override { return true; }
    CommandDispatchResult handle(const Command& command, CommandResult& output) const override
    {
        ++calls; order = ++sequence;
        if (result == CommandDispatchResult::SUCCESS) output = successResult(command);
        return result;
    }
};

void drive(CommandExecutor& executor, uint32_t nowMs)
{
    for (uint8_t index = 0U; index < 5U; ++index) executor.update(nowMs);
}

struct ExecutorFixture
{
    SceneExecutionQueue queue;
    CommandValidator validator;
    FakeGate gate;
    FakeCommitHook commit;
    FakeHandler handler;
    CommandDispatcher dispatcher;
    CommandExecutor executor;
    ExecutorFixture() : executor(queue, validator, gate, commit, dispatcher)
    { dispatcher.registerHandler(handler); sequence = 0U; }
};

class Mode final : public SystemModeProvider { public: mutable uint8_t calls=0U; SystemModeCheckResult check(const Command&,uint32_t)const override{++calls;return SystemModeCheckResult::ALLOWED;} };
class Auth final : public AuthorizationPolicy { public: mutable uint8_t calls=0U; AuthorizationCheckResult check(const Command&,uint32_t)const override{++calls;return AuthorizationCheckResult::ALLOWED;} };
class Safety final : public SafetyPolicy { public: mutable uint8_t calls=0U; SafetyCheckResult check(const Command&,uint32_t)const override{++calls;return SafetyCheckResult::ALLOWED;} };
class Token final : public ConfirmTokenProvider
{
public:
    ConfirmTokenCheckResult checked=ConfirmTokenCheckResult::NOT_REQUIRED;
    ConfirmTokenCheckResult consumed=ConfirmTokenCheckResult::NOT_REQUIRED;
    mutable uint8_t checks=0U; uint8_t consumes=0U;
    ConfirmTokenCheckResult check(const Command&,uint32_t)const override{++checks;return checked;}
    ConfirmTokenCheckResult consume(const Command&,uint32_t)override{++consumes;return consumed;}
};
}

void setUp() {}
void tearDown() {}

void test_success_order_and_exactly_once()
{
    ExecutorFixture f; const uint32_t now = 77U; f.queue.enqueue(makeCommand()); drive(f.executor, now);
    TEST_ASSERT_EQUAL_UINT8(1U,f.gate.calls); TEST_ASSERT_EQUAL_UINT8(1U,f.handler.calls); TEST_ASSERT_EQUAL_UINT8(1U,f.commit.calls);
    TEST_ASSERT_EQUAL_UINT8(1U,f.gate.order); TEST_ASSERT_EQUAL_UINT8(2U,f.handler.order); TEST_ASSERT_EQUAL_UINT8(3U,f.commit.order);
    TEST_ASSERT_EQUAL_UINT32(now,f.gate.now); TEST_ASSERT_EQUAL_UINT32(now,f.commit.now);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutorResult::SUCCESS,(uint8_t)f.executor.getLastResult());
}

void test_gate_and_dispatch_failures_short_circuit()
{
    ExecutorFixture f; f.gate.result=CommandExecutionGateResult::UNAUTHORIZED; f.queue.enqueue(makeCommand()); drive(f.executor,1U);
    TEST_ASSERT_EQUAL_UINT8(1U,f.gate.calls); TEST_ASSERT_EQUAL_UINT8(0U,f.handler.calls); TEST_ASSERT_EQUAL_UINT8(0U,f.commit.calls);
    f.executor.reset(); f.gate.result=CommandExecutionGateResult::ALLOWED; f.handler.result=CommandDispatchResult::HANDLER_REJECTED; f.queue.enqueue(makeCommand()); drive(f.executor,2U);
    TEST_ASSERT_EQUAL_UINT8(1U,f.handler.calls); TEST_ASSERT_EQUAL_UINT8(0U,f.commit.calls);
}

void test_commit_failures_are_fail_closed_without_retry()
{
    const CommandExecutionGateResult failures[]={CommandExecutionGateResult::INVALID_COMMAND,CommandExecutionGateResult::SYSTEM_MODE_REJECTED,CommandExecutionGateResult::UNAUTHORIZED,CommandExecutionGateResult::CONFIRMATION_REQUIRED,CommandExecutionGateResult::INVALID_CONFIRM_TOKEN,CommandExecutionGateResult::SAFETY_REJECTED,CommandExecutionGateResult::POLICY_ERROR,static_cast<CommandExecutionGateResult>(99U)};
    for(auto failure:failures){ExecutorFixture f;f.commit.result=failure;f.queue.enqueue(makeCommand());drive(f.executor,9U);TEST_ASSERT_EQUAL_UINT8(1U,f.commit.calls);TEST_ASSERT_EQUAL_UINT8((uint8_t)CommandExecutorState::FAILED,(uint8_t)f.executor.getState());TEST_ASSERT_FALSE(f.executor.getLastCommandResult()->isSuccess());}
}

void test_real_composite_consumes_only_after_terminal_success()
{
    Mode mode;Auth auth;Safety safety;Token token;CompositeCommandExecutionGate composite(mode,auth,safety,token);
    SceneExecutionQueue queue;CommandValidator validator;FakeHandler handler;CommandDispatcher dispatcher;dispatcher.registerHandler(handler);
    CommandExecutor executor(queue,validator,composite,composite,dispatcher);
    token.checked=ConfirmTokenCheckResult::VALID;token.consumed=ConfirmTokenCheckResult::VALID;queue.enqueue(makeCommand(CommandRisk::DANGEROUS));drive(executor,5U);
    TEST_ASSERT_EQUAL_UINT8(1U,token.consumes);TEST_ASSERT_EQUAL_UINT8(1U,mode.calls);TEST_ASSERT_EQUAL_UINT8(1U,auth.calls);TEST_ASSERT_EQUAL_UINT8(1U,safety.calls);
    executor.reset();handler.result=CommandDispatchResult::HANDLER_REJECTED;queue.enqueue(makeCommand(CommandRisk::DANGEROUS));drive(executor,6U);TEST_ASSERT_EQUAL_UINT8(1U,token.consumes);
}

void setup(){UNITY_BEGIN();RUN_TEST(test_success_order_and_exactly_once);RUN_TEST(test_gate_and_dispatch_failures_short_circuit);RUN_TEST(test_commit_failures_are_fail_closed_without_retry);RUN_TEST(test_real_composite_consumes_only_after_terminal_success);UNITY_END();}
void loop() {}
