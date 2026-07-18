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
        result.context.request.source = static_cast<CommandSource>(5U);
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

    CommandResult makeFailedResult(const Command& command)
    {
        CommandResult result;
        result.commandId = command.context.commandId;
        result.requestId = command.context.request.requestId;
        result.transitionTo(ExecutionStatus::VALIDATING, 1U);
        result.transitionTo(ExecutionStatus::ACCEPTED, 2U);
        result.transitionTo(ExecutionStatus::EXECUTING, 3U);
        result.transitionTo(ExecutionStatus::FAILED, 4U,
                            CommandErrorCode::HARDWARE_FAILURE);
        return result;
    }

    enum class FakeResultMode : uint8_t
    {
        CORRECT,
        WRONG_COMMAND_ID,
        WRONG_REQUEST_ID,
        FAILED_RESULT
    };

    class FakeHandler final : public CommandHandler
    {
    public:
        CommandDomain domain = CommandDomain::OUT;
        CommandDispatchResult result = CommandDispatchResult::SUCCESS;
        FakeResultMode resultMode = FakeResultMode::CORRECT;
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
            output = resultMode == FakeResultMode::FAILED_RESULT
                ? makeFailedResult(command) : makeSuccessResult(command);
            if (resultMode == FakeResultMode::WRONG_COMMAND_ID)
                ++output.commandId;
            else if (resultMode == FakeResultMode::WRONG_REQUEST_ID)
                ++output.requestId;
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

void test_dispatcher_registration_and_ordered_removal()
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

void test_dispatcher_preserves_registration_order()
{
    CommandDispatcher dispatcher;
    FakeHandler h1;
    FakeHandler h2;
    FakeHandler h3;
    dispatcher.registerHandler(h1);
    dispatcher.registerHandler(h2);
    dispatcher.unregisterHandler(h1);
    dispatcher.registerHandler(h3);
    CommandResult output;
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(dispatcher.dispatch(makeCommand(), output)));
    TEST_ASSERT_EQUAL_UINT16(1U, h2.handleCount);
    TEST_ASSERT_EQUAL_UINT16(0U, h3.handleCount);

    dispatcher.clear();
    h1.handleCount = h2.handleCount = h3.handleCount = 0U;
    dispatcher.registerHandler(h1);
    dispatcher.registerHandler(h2);
    dispatcher.registerHandler(h3);
    dispatcher.unregisterHandler(h2);
    dispatcher.unregisterHandler(h1);
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(dispatcher.dispatch(makeCommand(), output)));
    TEST_ASSERT_EQUAL_UINT16(1U, h3.handleCount);
}

void test_dispatcher_rejects_invalid_success_results_atomically()
{
    CommandDispatcher dispatcher;
    FakeHandler handler;
    dispatcher.registerHandler(handler);
    const FakeResultMode modes[] = {
        FakeResultMode::WRONG_COMMAND_ID,
        FakeResultMode::WRONG_REQUEST_ID,
        FakeResultMode::FAILED_RESULT
    };
    for (FakeResultMode mode : modes)
    {
        handler.resultMode = mode;
        CommandResult output;
        output.commandId = 99U;
        TEST_ASSERT_EQUAL_UINT8(
            static_cast<uint8_t>(CommandDispatchResult::RESULT_INVALID),
            static_cast<uint8_t>(dispatcher.dispatch(makeCommand(), output)));
        TEST_ASSERT_EQUAL_UINT32(99U, output.commandId);
    }
    handler.resultMode = FakeResultMode::CORRECT;
    CommandResult output;
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(dispatcher.dispatch(makeCommand(), output)));
    TEST_ASSERT_TRUE(output.isSuccess());
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

void test_executor_rejects_bad_result_without_reexecution()
{
    SceneExecutionQueue queue;
    CommandValidator validator;
    FakeGate gate;
    FakeHandler handler;
    handler.resultMode = FakeResultMode::WRONG_COMMAND_ID;
    CommandDispatcher dispatcher;
    dispatcher.registerHandler(handler);
    CommandExecutor executor(queue, validator, gate, dispatcher);
    const Command command = makeCommand();
    queue.enqueue(command);
    for (uint32_t now = 1U; now <= 5U; ++now)
        executor.update(now);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandExecutorState::FAILED),
        static_cast<uint8_t>(executor.getState()));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandExecutorResult::COMMAND_RESULT_INVALID),
        static_cast<uint8_t>(executor.getLastResult()));
    TEST_ASSERT_EQUAL_UINT16(1U, handler.handleCount);
    const CommandResult* result = executor.getLastCommandResult();
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_UINT32(command.context.commandId, result->commandId);
    TEST_ASSERT_EQUAL_UINT32(command.context.request.requestId, result->requestId);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(ExecutionStatus::FAILED),
        static_cast<uint8_t>(result->status));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandErrorCode::HARDWARE_FAILURE),
        static_cast<uint8_t>(result->errorCode));
    executor.update(6U);
    TEST_ASSERT_EQUAL_UINT16(1U, handler.handleCount);
}

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_dispatcher_registration_and_ordered_removal);
    RUN_TEST(test_dispatcher_routes_once_and_is_atomic);
    RUN_TEST(test_dispatcher_preserves_registration_order);
    RUN_TEST(test_dispatcher_rejects_invalid_success_results_atomically);
    RUN_TEST(test_executor_success_state_machine);
    RUN_TEST(test_executor_gate_and_dispatch_failures);
    RUN_TEST(test_executor_rejects_bad_result_without_reexecution);
    UNITY_END();
}

void loop() {}
