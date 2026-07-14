#include <RuleExecutor.h>
#include <SceneExecutionQueueSink.h>
#include <Arduino.h>
#include <unity.h>

class FakeCommandIdProvider final : public CommandIdProvider
{
public:
    CommandIdReservationResult result;
    CommandId first;
    size_t calls;
    size_t lastCount;
    FakeCommandIdProvider() : result(CommandIdReservationResult::SUCCESS), first(100U),
        calls(0U), lastCount(0U) {}
    CommandIdReservationResult reserveRange(size_t count, CommandId& output) override
    {
        ++calls; lastCount = count;
        if (result == CommandIdReservationResult::SUCCESS) output = first;
        return result;
    }
};

static RuleManager managerValue;
static RuleTriggerQueue triggerQueueValue;
static AutomationCommandFactory factoryValue;
static FakeCommandIdProvider idProviderValue;
static SceneExecutionQueue commandQueueValue;
static SceneExecutionQueueSink sinkValue(commandQueueValue);
static RuleExecutor executorValue(managerValue, triggerQueueValue, factoryValue,
    idProviderValue, sinkValue);

static ConditionExpression makeCondition()
{
    ConditionComparison comparison;
    comparison.left = ConditionOperand::makeBoolean(true);
    comparison.comparisonOperator = ComparisonOperator::EQUAL;
    comparison.right = ConditionOperand::makeBoolean(true);
    ConditionExpression expression;
    expression.addFirst(comparison);
    return expression;
}

static AutomationCommand makeCommand(uint16_t index, uint32_t durationMs = 0U)
{
    AutomationCommand command;
    command.domain = CommandDomain::OUT;
    command.domainIndex = index;
    command.hasDomainIndex = true;
    command.operation = CommandOperation::ON;
    if (durationMs > 0U) command.setDuration(durationMs);
    return command;
}

static RuleActionStep makeStep(AutomationStepIndex index, RuleBranch branch,
    bool enabled = true, uint32_t delayMs = 0U)
{
    RuleActionStep step;
    step.stepIndex = index;
    step.branch = branch;
    step.command = makeCommand(index);
    step.enabled = enabled;
    step.delayBeforeMs = delayMs;
    return step;
}

static Rule makeRule(RuleId id, bool enabled = true)
{
    Rule rule;
    rule.id = id;
    rule.setName(id == 1U ? "Rule One" : "Rule Two");
    rule.setCondition(makeCondition());
    rule.enabled = enabled;
    return rule;
}

static RuleTrigger makeTrigger(RuleId id, RuleBranch branch)
{
    RuleTrigger trigger;
    trigger.ruleId = id;
    trigger.branch = branch;
    trigger.triggerType = branch == RuleBranch::THEN_BRANCH ?
        RuleTriggerType::RISING_EDGE : RuleTriggerType::FALLING_EDGE;
    trigger.request.requestId = id;
    trigger.request.source = CommandSource::RULE_ENGINE;
    return trigger;
}

static void resetFixture()
{
    executorValue.reset();
    managerValue.clear();
    triggerQueueValue.clear();
    commandQueueValue.clear();
    idProviderValue = FakeCommandIdProvider{};
}

static void advanceToReady(uint32_t nowMs = 0U)
{
    executorValue.update(nowMs);
    executorValue.update(nowMs);
    executorValue.update(nowMs);
}

void testAdapterMappingAndOwnership()
{
    resetFixture();
    Command invalid;
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RuntimeCommandSubmitResult::INVALID_COMMAND),
        static_cast<uint8_t>(sinkValue.submit(invalid)));
    Rule rule = makeRule(1U); rule.addActionStep(makeStep(1U, RuleBranch::THEN_BRANCH));
    managerValue.add(rule); triggerQueueValue.enqueue(makeTrigger(1U, RuleBranch::THEN_BRANCH));
    advanceToReady(); executorValue.update(10U);
    TEST_ASSERT_EQUAL_UINT32(1U, commandQueueValue.size());
    TEST_ASSERT_EQUAL_UINT32(100U, commandQueueValue.peek()->context.commandId);
    TEST_ASSERT_FALSE(commandQueueValue.isEmpty());
    while (!commandQueueValue.isFull()) commandQueueValue.enqueue(*commandQueueValue.peek());
    TEST_ASSERT_TRUE(sinkValue.isFull());
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RuntimeCommandSubmitResult::SINK_FULL),
        static_cast<uint8_t>(sinkValue.submit(*commandQueueValue.peek())));
}

void testEmptyValidationAndReservation()
{
    resetFixture(); executorValue.update(0U);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RuleExecutionState::IDLE),
        static_cast<uint8_t>(executorValue.getState()));
    triggerQueueValue.enqueue(makeTrigger(9U, RuleBranch::THEN_BRANCH)); advanceToReady();
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RuleExecutionResult::RULE_NOT_FOUND),
        static_cast<uint8_t>(executorValue.getLastResult()));

    resetFixture(); Rule empty = makeRule(1U); managerValue.add(empty);
    triggerQueueValue.enqueue(makeTrigger(1U, RuleBranch::THEN_BRANCH));
    executorValue.update(0U); executorValue.update(0U);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RuleExecutionState::COMPLETED),
        static_cast<uint8_t>(executorValue.getState()));
    TEST_ASSERT_EQUAL_UINT32(0U, idProviderValue.calls);

    resetFixture(); Rule disabled = makeRule(1U, false); managerValue.add(disabled);
    triggerQueueValue.enqueue(makeTrigger(1U, RuleBranch::THEN_BRANCH));
    executorValue.update(0U); executorValue.update(0U);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RuleExecutionResult::RULE_DISABLED),
        static_cast<uint8_t>(executorValue.getLastResult()));
}

void testEnabledStepsDelayAndMapping()
{
    resetFixture(); Rule rule = makeRule(1U);
    rule.addActionStep(makeStep(1U, RuleBranch::THEN_BRANCH, false, 999U));
    RuleActionStep active = makeStep(2U, RuleBranch::THEN_BRANCH, true, 5U);
    active.command = makeCommand(2U, 30000U); rule.addActionStep(active); managerValue.add(rule);
    triggerQueueValue.enqueue(makeTrigger(1U, RuleBranch::THEN_BRANCH)); advanceToReady(0xFFFFFFFEU);
    executorValue.update(0xFFFFFFFEU);
    TEST_ASSERT_EQUAL_UINT32(1U, idProviderValue.lastCount);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RuleExecutionState::WAITING_DELAY),
        static_cast<uint8_t>(executorValue.getState()));
    executorValue.update(2U); TEST_ASSERT_TRUE(commandQueueValue.isEmpty());
    executorValue.update(3U);
    TEST_ASSERT_EQUAL_UINT32(1U, commandQueueValue.size());
    const Command* command = commandQueueValue.peek();
    TEST_ASSERT_EQUAL_UINT32(100U, command->context.commandId);
    TEST_ASSERT_EQUAL_UINT32(3U, command->context.createdTimestampMs);
    TEST_ASSERT_EQUAL_UINT32(30000U, command->durationMs);
    TEST_ASSERT_EQUAL_UINT32(1U, command->context.request.requestId);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandSource::RULE_ENGINE),
        static_cast<uint8_t>(command->context.request.source));
    TEST_ASSERT_EQUAL_INT8(0, command->originalText[0]);
}

void testBackpressureRetryAndCancellation()
{
    resetFixture(); Rule rule = makeRule(1U);
    rule.addActionStep(makeStep(1U, RuleBranch::THEN_BRANCH)); managerValue.add(rule);
    Command filler; RequestContext request; request.requestId = 50U; request.source = CommandSource::RULE_ENGINE;
    AutomationCommand source = makeCommand(1U);
    for (CommandId id = 1U; id <= SCENE_COMMAND_QUEUE_CAPACITY; ++id)
    { factoryValue.create(source, request, id, 0U, filler); commandQueueValue.enqueue(filler); }
    triggerQueueValue.enqueue(makeTrigger(1U, RuleBranch::THEN_BRANCH)); advanceToReady();
    executorValue.update(0U);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RuleExecutionState::WAITING_COMMAND_SINK),
        static_cast<uint8_t>(executorValue.getState()));
    TEST_ASSERT_EQUAL_UINT16(1U, executorValue.getCurrentStepIndex());
    commandQueueValue.consume(); executorValue.update(9U);
    TEST_ASSERT_EQUAL_UINT32(100U, commandQueueValue.getAt(commandQueueValue.size()-1U)->context.commandId);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RuleExecutionState::COMPLETED),
        static_cast<uint8_t>(executorValue.getState()));

    executorValue.update(10U); triggerQueueValue.enqueue(makeTrigger(1U, RuleBranch::THEN_BRANCH));
    advanceToReady();
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RuleExecutionResult::CANCELLED),
        static_cast<uint8_t>(executorValue.cancelCurrent()));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RuleExecutionState::CANCELLED),
        static_cast<uint8_t>(executorValue.getState()));
}

void testReservationFailuresAndRuleChange()
{
    resetFixture(); Rule rule = makeRule(1U);
    rule.addActionStep(makeStep(1U, RuleBranch::THEN_BRANCH)); managerValue.add(rule);
    triggerQueueValue.enqueue(makeTrigger(1U, RuleBranch::THEN_BRANCH));
    idProviderValue.result = CommandIdReservationResult::RANGE_UNAVAILABLE; advanceToReady();
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RuleExecutionResult::COMMAND_ID_RESERVATION_FAILED),
        static_cast<uint8_t>(executorValue.getLastResult()));
    TEST_ASSERT_TRUE(commandQueueValue.isEmpty());

    resetFixture(); managerValue.add(rule); triggerQueueValue.enqueue(makeTrigger(1U, RuleBranch::THEN_BRANCH));
    idProviderValue.first = 0U; advanceToReady();
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RuleExecutionResult::INVALID_COMMAND_ID_RANGE),
        static_cast<uint8_t>(executorValue.getLastResult()));

    resetFixture(); rule.addActionStep(makeStep(2U, RuleBranch::THEN_BRANCH)); managerValue.add(rule);
    triggerQueueValue.enqueue(makeTrigger(1U, RuleBranch::THEN_BRANCH)); idProviderValue.first = UINT32_MAX;
    advanceToReady();
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RuleExecutionResult::INVALID_COMMAND_ID_RANGE),
        static_cast<uint8_t>(executorValue.getLastResult()));

    resetFixture(); rule = makeRule(1U); rule.addActionStep(makeStep(1U, RuleBranch::THEN_BRANCH));
    managerValue.add(rule); triggerQueueValue.enqueue(makeTrigger(1U, RuleBranch::THEN_BRANCH));
    advanceToReady(); managerValue.remove(1U); executorValue.update(0U);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RuleExecutionResult::RULE_NOT_FOUND),
        static_cast<uint8_t>(executorValue.getLastResult()));
}

static_assert(sizeof(SceneExecutionQueueSink) <= 8U, "اندازه‌ی Adapter از هدف بیشتر است");
static_assert(sizeof(RuleExecutor) < 256U, "اندازه‌ی RuleExecutor از هدف بیشتر است");

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(testAdapterMappingAndOwnership);
    RUN_TEST(testEmptyValidationAndReservation);
    RUN_TEST(testEnabledStepsDelayAndMapping);
    RUN_TEST(testBackpressureRetryAndCancellation);
    RUN_TEST(testReservationFailuresAndRuleChange);
    UNITY_END();
}
void loop() {}
