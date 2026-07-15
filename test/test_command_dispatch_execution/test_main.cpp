#include <CommandExecutor.h>
#include <Arduino.h>
#include <unity.h>

namespace
{
    Command makeCommand(CommandDomain domain = CommandDomain::OUT)
    {
        Command result;
        result.context.commandId = 10U;
        result.context.request.requestId = 20U;
        result.context.request.source = CommandSource::SERIAL;
        result.context.createdTimestampMs = 30U;
        result.domain = domain;
        result.domainIndex = 1U;
        result.hasDomainIndex = true;
        result.operation = CommandOperation::ON;
        return result;
    }

    CommandResult makeSuccessResult(const Command& command)
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

    class FakeHandler final : public CommandHandler
    {
    public:
        CommandDomain domain = CommandDomain::OUT;
        CommandDispatchResult result = CommandDispatchResult::SUCCESS;
        mutable uint16_t supportsCount = 0U;
        mutable uint16_t handleCount = 0U;

        bool supports(const Command& command) const override
        {
            ++supportsCount;
            return command.domain == domain;
        }
        CommandDispatchResult handle(
            const Command& command,
            CommandResult& output
        ) const override
        {
            ++handleCount;
            if (result != CommandDispatchResult::SUCCESS)
                return result;
            output = makeSuccessResult(command);
            return CommandDispatchResult::SUCCESS;
        }
    };

    class FakeGate final : public CommandExecutionGate
    {
    public:
        CommandExecutionGateResult result = CommandExecutionGateResult::ALLOWED;
        mutable uint16_t checkCount = 0U;
        mutable uint32_t lastNowMs = 0U;
        CommandExecutionGateResult check(const Command&, uint32_t nowMs) const override
        {
            ++checkCount;
            lastNowMs = nowMs;
            return result;
        }
    };
}

void setUp() {}
void tearDown() {}

void test_dispatcher_registration_and_sparse_removal()
{
    CommandDispatcher dispatcher;
    FakeHandler handlers[9];
    TEST_ASSERT_EQUAL_UINT32(0U, dispatcher.size());
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(dispatcher.registerHandler(handlers[0])));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandDispatchResult::HANDLER_ALREADY_REGISTERED),
        static_cast<uint8_t>(dispatcher.registerHandler(handlers[0]))
    );
    for (size_t index = 1U; index < 8U; ++index)
        TEST_ASSERT_EQUAL_UINT8(0U,
            static_cast<uint8_t>(dispatcher.registerHandler(handlers[index])));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandDispatchResult::DISPATCHER_FULL),
        static_cast<uint8_t>(dispatcher.registerHandler(handlers[8]))
    );
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(dispatcher.unregisterHandler(handlers[3])));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandDispatchResult::HANDLER_NOT_REGISTERED),
        static_cast<uint8_t>(dispatcher.unregisterHandler(handlers[3]))
    );
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(dispatcher.registerHandler(handlers[8])));
}

void test_dispatcher_routes_once_and_is_atomic()
{
    CommandDispatcher dispatcher;
    FakeHandler first;
    FakeHandler second;
    dispatcher.registerHandler(first);
    dispatcher.registerHandler(second);
    CommandResult output;
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(dispatcher.dispatch(makeCommand(), output)));
    TEST_ASSERT_EQUAL_UINT16(1U, first.handleCount);
    TEST_ASSERT_EQUAL_UINT16(0U, second.handleCount);
    TEST_ASSERT_TRUE(output.isSuccess());

    first.result = CommandDispatchResult::HANDLER_REJECTED;
    output.commandId = 99U;
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandDispatchResult::HANDLER_REJECTED),
        static_cast<uint8_t>(dispatcher.dispatch(makeCommand(), output))
    );
    TEST_ASSERT_EQUAL_UINT32(99U, output.commandId);
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandDispatchResult::HANDLER_NOT_FOUND),
        static_cast<uint8_t>(dispatcher.dispatch(makeCommand(CommandDomain::NODE), output))
    );
}

void test_executor_success_state_machine()
{
    SceneExecutionQueue queue;
    CommandValidator validator;
    FakeGate gate;
    FakeHandler handler;
    CommandDispatcher dispatcher;
    dispatcher.registerHandler(handler);
    CommandExecutor executor(queue, validator, gate, dispatcher);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandExecutorState::IDLE),
        static_cast<uint8_t>(executor.getState()));
    executor.update(1U);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandExecutorResult::COMMAND_QUEUE_EMPTY),
        static_cast<uint8_t>(executor.getLastResult()));
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(queue.enqueue(makeCommand())));
    executor.update(2U);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandExecutorState::LOADING_COMMAND),
        static_cast<uint8_t>(executor.getState()));
    TEST_ASSERT_TRUE(queue.isEmpty());
    executor.update(3U);
    executor.update(4U);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandExecutorState::CHECKING_GATE),
        static_cast<uint8_t>(executor.getState()));
    executor.update(5U);
    TEST_ASSERT_EQUAL_UINT16(1U, gate.checkCount);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandExecutorState::DISPATCHING),
        static_cast<uint8_t>(executor.getState()));
    executor.update(6U);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandExecutorState::COMPLETED),
        static_cast<uint8_t>(executor.getState()));
    TEST_ASSERT_TRUE(executor.hasLastCommandResult());
    TEST_ASSERT_TRUE(executor.getLastCommandResult()->isSuccess());
    executor.update(7U);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandExecutorState::IDLE),
        static_cast<uint8_t>(executor.getState()));
}

void test_executor_gate_and_dispatch_failures()
{
    SceneExecutionQueue queue;
    CommandValidator validator;
    FakeGate gate;
    FakeHandler handler;
    CommandDispatcher dispatcher;
    dispatcher.registerHandler(handler);
    CommandExecutor executor(queue, validator, gate, dispatcher);
    queue.enqueue(makeCommand());
    gate.result = CommandExecutionGateResult::UNAUTHORIZED;
    executor.update(1U);
    executor.update(2U);
    executor.update(3U);
    executor.update(4U);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandExecutorState::FAILED),
        static_cast<uint8_t>(executor.getState()));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandErrorCode::UNAUTHORIZED),
        static_cast<uint8_t>(executor.getLastCommandResult()->errorCode));
    TEST_ASSERT_EQUAL_UINT16(0U, handler.handleCount);

    executor.reset();
    gate.result = CommandExecutionGateResult::ALLOWED;
    handler.result = CommandDispatchResult::HANDLER_REJECTED;
    queue.enqueue(makeCommand());
    executor.update(5U);
    executor.update(6U);
    executor.update(7U);
    executor.update(8U);
    executor.update(9U);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandExecutorState::FAILED),
        static_cast<uint8_t>(executor.getState()));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandErrorCode::HARDWARE_FAILURE),
        static_cast<uint8_t>(executor.getLastCommandResult()->errorCode));
    TEST_ASSERT_TRUE(queue.isEmpty());
    executor.clearLastCommandResult();
    TEST_ASSERT_FALSE(executor.hasLastCommandResult());
}

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_dispatcher_registration_and_sparse_removal);
    RUN_TEST(test_dispatcher_routes_once_and_is_atomic);
    RUN_TEST(test_executor_success_state_machine);
    RUN_TEST(test_executor_gate_and_dispatch_failures);
    UNITY_END();
}

void loop() {}
