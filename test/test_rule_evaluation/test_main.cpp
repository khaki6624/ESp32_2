#include <Arduino.h>
#ifdef INPUT
#undef INPUT
#endif
#include <ConditionEvaluator.h>
#include <RuleEngine.h>
#include <type_traits>
#include <unity.h>

class FakeProvider:public ConditionValueProvider
{
public:
    mutable size_t calls;ConditionResolveResult result;ConditionResolvedValue value;
    FakeProvider():calls(0),result(ConditionResolveResult::SUCCESS),value(ConditionResolvedValue::makeBoolean(false)){}
    ConditionResolveResult resolve(const ConditionOperand&,ConditionResolvedValue& output)const override
    {++calls;if(result==ConditionResolveResult::SUCCESS)output=value;return result;}
};

static FakeProvider providerValue;
static ConditionEvaluator evaluatorValue(providerValue);
static RuleManager managerValue;
static RuleTriggerQueue queueValue;
static RuleEngine engineValue(managerValue,evaluatorValue,queueValue);

static RequestContext makeRequest(RequestId id=1)
{RequestContext r;r.requestId=id;r.source=CommandSource::RULE_ENGINE;return r;}
static ConditionComparison makeComparison(const ConditionOperand& left,ComparisonOperator op,const ConditionOperand& right)
{ConditionComparison c;c.left=left;c.comparisonOperator=op;c.right=right;return c;}
static ConditionExpression makeBooleanExpression(bool value)
{ConditionExpression e;e.addFirst(makeComparison(ConditionOperand::makeBoolean(value),ComparisonOperator::EQUAL,ConditionOperand::makeBoolean(true)));return e;}
static Rule makeRule(RuleId id,bool condition,bool enabled=true)
{Rule r;r.id=id;r.setName(id==1?"Rule One":"Rule Other");r.setCondition(makeBooleanExpression(condition));r.enabled=enabled;return r;}
static void resetFixture()
{providerValue=FakeProvider{};managerValue.clear();queueValue.clear();engineValue.resetRuntimeStates();}

void testResolvedValue()
{
    ConditionResolvedValue value;TEST_ASSERT_FALSE(value.isValid());
    TEST_ASSERT_TRUE(ConditionResolvedValue::makeBoolean(true).isValid());TEST_ASSERT_TRUE(ConditionResolvedValue::makeInteger(-2).isValid());
    TEST_ASSERT_TRUE(ConditionResolvedValue::makeFloat(1.5F).isValid());TEST_ASSERT_FALSE(ConditionResolvedValue::makeFloat(NAN).isValid());TEST_ASSERT_FALSE(ConditionResolvedValue::makeFloat(INFINITY).isValid());
    TEST_ASSERT_TRUE(ConditionResolvedValue::makePercentage(0).isValid());TEST_ASSERT_TRUE(ConditionResolvedValue::makePercentage(100).isValid());TEST_ASSERT_FALSE(ConditionResolvedValue::makePercentage(-1).isValid());TEST_ASSERT_FALSE(ConditionResolvedValue::makePercentage(101).isValid());
    value=ConditionResolvedValue::makeEnum(3);value.clear();TEST_ASSERT_FALSE(value.isValid());
}

void testProviderAndComparisonPolicies()
{
    resetFixture();ConditionEvaluationResult output;ConditionComparison comparison=makeComparison(ConditionOperand::makeInteger(4),ComparisonOperator::GREATER_THAN,ConditionOperand::makeInteger(3));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(ConditionEvaluatorResult::SUCCESS),static_cast<uint8_t>(evaluatorValue.evaluateComparison(comparison,output)));TEST_ASSERT_EQUAL_UINT32(0,providerValue.calls);TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(ConditionEvaluationResult::TRUE_RESULT),static_cast<uint8_t>(output));
    comparison=makeComparison(ConditionOperand::makeBoolean(true),ComparisonOperator::LESS_THAN,ConditionOperand::makeBoolean(false));TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(ConditionEvaluatorResult::UNSUPPORTED_OPERATOR),static_cast<uint8_t>(evaluatorValue.evaluateComparison(comparison,output)));
    comparison=makeComparison(ConditionOperand::makeEnum(1),ComparisonOperator::NOT_EQUAL,ConditionOperand::makeEnum(2));TEST_ASSERT_TRUE(evaluatorValue.evaluateComparison(comparison,output)==ConditionEvaluatorResult::SUCCESS);TEST_ASSERT_TRUE(output==ConditionEvaluationResult::TRUE_RESULT);
    ConditionOperand reference=ConditionOperand::makeReference(ConditionSourceType::INPUT,1,ConditionValueType::BOOLEAN);comparison=makeComparison(reference,ComparisonOperator::EQUAL,ConditionOperand::makeBoolean(true));providerValue.value=ConditionResolvedValue::makeBoolean(true);
    TEST_ASSERT_TRUE(evaluatorValue.evaluateComparison(comparison,output)==ConditionEvaluatorResult::SUCCESS);TEST_ASSERT_EQUAL_UINT32(1,providerValue.calls);
    providerValue.result=ConditionResolveResult::SOURCE_OFFLINE;ConditionResolvedValue unchanged=ConditionResolvedValue::makeInteger(9);providerValue.resolve(reference,unchanged);TEST_ASSERT_EQUAL_INT32(9,unchanged.integerValue);
    TEST_ASSERT_TRUE(evaluatorValue.evaluateComparison(comparison,output)==ConditionEvaluatorResult::RESOLVE_FAILED);TEST_ASSERT_TRUE(output==ConditionEvaluationResult::ERROR_RESULT);
    providerValue.result=ConditionResolveResult::SUCCESS;providerValue.value=ConditionResolvedValue::makeInteger(1);TEST_ASSERT_TRUE(evaluatorValue.evaluateComparison(comparison,output)==ConditionEvaluatorResult::TYPE_MISMATCH);
}

void testExpressionPrecedenceAndNoShortCircuit()
{
    resetFixture();ConditionExpression expression;expression.addFirst(makeComparison(ConditionOperand::makeBoolean(true),ComparisonOperator::EQUAL,ConditionOperand::makeBoolean(true)));
    expression.add(LogicalOperator::OR,makeComparison(ConditionOperand::makeBoolean(false),ComparisonOperator::EQUAL,ConditionOperand::makeBoolean(true)));
    expression.add(LogicalOperator::AND,makeComparison(ConditionOperand::makeBoolean(false),ComparisonOperator::EQUAL,ConditionOperand::makeBoolean(true)));
    ConditionEvaluationResult output;TEST_ASSERT_TRUE(evaluatorValue.evaluate(expression,output)==ConditionEvaluatorResult::SUCCESS);TEST_ASSERT_TRUE(output==ConditionEvaluationResult::TRUE_RESULT);
    ConditionExpression invalid;TEST_ASSERT_TRUE(evaluatorValue.evaluate(invalid,output)==ConditionEvaluatorResult::INVALID_EXPRESSION);TEST_ASSERT_TRUE(output==ConditionEvaluationResult::ERROR_RESULT);
    ConditionOperand reference=ConditionOperand::makeReference(ConditionSourceType::INPUT,1,ConditionValueType::BOOLEAN);expression.clear();expression.addFirst(makeComparison(ConditionOperand::makeBoolean(true),ComparisonOperator::EQUAL,ConditionOperand::makeBoolean(true)));expression.add(LogicalOperator::OR,makeComparison(reference,ComparisonOperator::EQUAL,ConditionOperand::makeBoolean(true)));providerValue.result=ConditionResolveResult::PROVIDER_ERROR;
    TEST_ASSERT_TRUE(evaluatorValue.evaluate(expression,output)==ConditionEvaluatorResult::RESOLVE_FAILED);TEST_ASSERT_EQUAL_UINT32(1,providerValue.calls);
}

static RuleTrigger makeTrigger(RuleId id,RuleTriggerType type)
{RuleTrigger t;t.ruleId=id;t.triggerType=type;t.branch=type==RuleTriggerType::RISING_EDGE?RuleBranch::THEN_BRANCH:RuleBranch::ELSE_BRANCH;t.request=makeRequest(id);return t;}
void testTriggerQueue()
{
    resetFixture();TEST_ASSERT_TRUE(queueValue.isEmpty());TEST_ASSERT_EQUAL_UINT32(8,queueValue.capacity());TEST_ASSERT_TRUE(queueValue.enqueue(RuleTrigger{})!=RuleEngineResult::SUCCESS);
    for(RuleId id=1;id<=8;++id)TEST_ASSERT_TRUE(queueValue.enqueue(makeTrigger(id,RuleTriggerType::RISING_EDGE))==RuleEngineResult::SUCCESS);
    TEST_ASSERT_TRUE(queueValue.isFull());const size_t before=queueValue.size();TEST_ASSERT_TRUE(queueValue.enqueue(makeTrigger(9,RuleTriggerType::RISING_EDGE))==RuleEngineResult::TRIGGER_QUEUE_FULL);TEST_ASSERT_EQUAL_UINT32(before,queueValue.size());
    TEST_ASSERT_EQUAL_UINT16(1,queueValue.peek()->ruleId);TEST_ASSERT_TRUE(queueValue.consume()==RuleEngineResult::SUCCESS);TEST_ASSERT_EQUAL_UINT16(2,queueValue.peek()->ruleId);queueValue.clear();TEST_ASSERT_TRUE(queueValue.consume()==RuleEngineResult::QUEUE_EMPTY);TEST_ASSERT_NULL(queueValue.peek());
}

void testBaselineEdgesAndNoRepetition()
{
    resetFixture();Rule rule=makeRule(1,false);TEST_ASSERT_TRUE(managerValue.add(rule)==AutomationModelResult::SUCCESS);RequestContext request=makeRequest(77);
    TEST_ASSERT_TRUE(engineValue.evaluateRule(1,10,request)==RuleEngineResult::SUCCESS);const RuleRuntimeState* state=engineValue.findRuntimeState(1);TEST_ASSERT_NOT_NULL(state);TEST_ASSERT_TRUE(state->initialized);TEST_ASSERT_TRUE(state->lastEvaluation==ConditionEvaluationResult::FALSE_RESULT);TEST_ASSERT_EQUAL_UINT32(10,state->lastEvaluationTimestampMs);TEST_ASSERT_EQUAL_UINT32(0,state->lastTransitionTimestampMs);TEST_ASSERT_TRUE(queueValue.isEmpty());
    rule.setCondition(makeBooleanExpression(true));managerValue.update(rule);TEST_ASSERT_TRUE(engineValue.evaluateRule(1,20,request)==RuleEngineResult::SUCCESS);TEST_ASSERT_EQUAL_UINT32(1,queueValue.size());TEST_ASSERT_TRUE(queueValue.peek()->triggerType==RuleTriggerType::RISING_EDGE);TEST_ASSERT_TRUE(queueValue.peek()->branch==RuleBranch::THEN_BRANCH);TEST_ASSERT_EQUAL_UINT32(77,queueValue.peek()->request.requestId);state=engineValue.findRuntimeState(1);TEST_ASSERT_TRUE(state->lastEvaluation==ConditionEvaluationResult::TRUE_RESULT);TEST_ASSERT_EQUAL_UINT32(20,state->lastTransitionTimestampMs);
    TEST_ASSERT_TRUE(engineValue.evaluateRule(1,21,request)==RuleEngineResult::SUCCESS);TEST_ASSERT_EQUAL_UINT32(1,queueValue.size());queueValue.consume();
    rule.setCondition(makeBooleanExpression(false));managerValue.update(rule);TEST_ASSERT_TRUE(engineValue.evaluateRule(1,30,request)==RuleEngineResult::SUCCESS);TEST_ASSERT_TRUE(queueValue.peek()->triggerType==RuleTriggerType::FALLING_EDGE);TEST_ASSERT_TRUE(queueValue.peek()->branch==RuleBranch::ELSE_BRANCH);
}

void testBackpressureErrorAndDisabledPolicies()
{
    resetFixture();Rule rule=makeRule(1,false);managerValue.add(rule);RequestContext request=makeRequest();engineValue.evaluateRule(1,1,request);
    for(RuleId id=2;id<=9;++id)queueValue.enqueue(makeTrigger(id,RuleTriggerType::RISING_EDGE));rule.setCondition(makeBooleanExpression(true));managerValue.update(rule);
    TEST_ASSERT_TRUE(engineValue.evaluateRule(1,2,request)==RuleEngineResult::TRIGGER_QUEUE_FULL);TEST_ASSERT_TRUE(engineValue.findRuntimeState(1)->lastEvaluation==ConditionEvaluationResult::FALSE_RESULT);queueValue.consume();TEST_ASSERT_TRUE(engineValue.evaluateRule(1,3,request)==RuleEngineResult::SUCCESS);TEST_ASSERT_TRUE(engineValue.findRuntimeState(1)->lastEvaluation==ConditionEvaluationResult::TRUE_RESULT);
    rule.enabled=false;managerValue.update(rule);TEST_ASSERT_TRUE(engineValue.evaluateRule(1,4,request)==RuleEngineResult::RULE_DISABLED);TEST_ASSERT_FALSE(engineValue.findRuntimeState(1)->initialized);queueValue.clear();rule.enabled=true;managerValue.update(rule);TEST_ASSERT_TRUE(engineValue.evaluateRule(1,5,request)==RuleEngineResult::SUCCESS);TEST_ASSERT_TRUE(queueValue.isEmpty());
}

void testRoundRobinAndStateByRuleId()
{
    resetFixture();managerValue.add(makeRule(1,false));managerValue.add(makeRule(2,true));RequestContext request=makeRequest();
    engineValue.update(1,request);TEST_ASSERT_EQUAL_UINT32(1,engineValue.getNextRuleArrayIndex());TEST_ASSERT_NOT_NULL(engineValue.findRuntimeState(1));TEST_ASSERT_NULL(engineValue.findRuntimeState(2));
    engineValue.update(2,request);TEST_ASSERT_EQUAL_UINT32(0,engineValue.getNextRuleArrayIndex());TEST_ASSERT_NOT_NULL(engineValue.findRuntimeState(2));managerValue.remove(1);engineValue.update(3,request);TEST_ASSERT_NOT_NULL(engineValue.findRuntimeState(2));TEST_ASSERT_NULL(engineValue.findRuntimeState(1));
    const size_t index=engineValue.getNextRuleArrayIndex();TEST_ASSERT_TRUE(engineValue.evaluateRule(99,4,request)==RuleEngineResult::RULE_NOT_FOUND);TEST_ASSERT_EQUAL_UINT32(index,engineValue.getNextRuleArrayIndex());
    managerValue.clear();engineValue.update(5,request);TEST_ASSERT_EQUAL_UINT32(0,engineValue.getNextRuleArrayIndex());TEST_ASSERT_EQUAL_UINT32(0,engineValue.runtimeStateCount());
}

static_assert(std::is_same<decltype(static_cast<const RuleTriggerQueue&>(queueValue).peek()),const RuleTrigger*>::value,"API صف Trigger باید فقط const باشد");
void setup(){UNITY_BEGIN();RUN_TEST(testResolvedValue);RUN_TEST(testProviderAndComparisonPolicies);RUN_TEST(testExpressionPrecedenceAndNoShortCircuit);RUN_TEST(testTriggerQueue);RUN_TEST(testBaselineEdgesAndNoRepetition);RUN_TEST(testBackpressureErrorAndDisabledPolicies);RUN_TEST(testRoundRobinAndStateByRuleId);UNITY_END();}
void loop(){}
