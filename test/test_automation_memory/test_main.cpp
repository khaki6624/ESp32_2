#include <SceneManager.h>
#include <RuleManager.h>
#include <ScheduleManager.h>
#include <Arduino.h>
#include <unity.h>
#include <math.h>
#include <type_traits>

static AutomationCommand makeAction(CommandDomain domain,uint16_t index,CommandOperation operation)
{
    AutomationCommand value;
    value.domain=domain;value.domainIndex=index;value.hasDomainIndex=index!=0U;value.operation=operation;
    return value;
}

static ConditionExpression makeCondition()
{
    ConditionComparison comparison;
    comparison.left=ConditionOperand::makeBoolean(true);
    comparison.comparisonOperator=ComparisonOperator::EQUAL;
    comparison.right=ConditionOperand::makeBoolean(true);
    ConditionExpression result;result.addFirst(comparison);return result;
}

static Scene makeScene(SceneId id,const char* name)
{Scene value;value.id=id;value.setName(name);return value;}
static Rule makeRule(RuleId id,const char* name)
{Rule value;value.id=id;value.setName(name);value.setCondition(makeCondition());return value;}
static Schedule makeSchedule(ScheduleId id,const char* name)
{Schedule value;value.id=id;value.setName(name);return value;}

void testArguments()
{
    AutomationCommandArgument empty;TEST_ASSERT_FALSE(empty.isValid());
    bool b=false;int32_t i=0;float f=0;uint8_t p=0;uint32_t u=1;
    auto boolean=AutomationCommandArgument::makeBoolean(true);TEST_ASSERT_TRUE(boolean.getBoolean(b));TEST_ASSERT_TRUE(b);TEST_ASSERT_FALSE(boolean.getInteger(i));
    TEST_ASSERT_TRUE(AutomationCommandArgument::makeInteger(-2).getInteger(i));
    TEST_ASSERT_TRUE(AutomationCommandArgument::makeFloat(1.5F).getFloat(f));
    TEST_ASSERT_FALSE(AutomationCommandArgument::makeFloat(NAN).isValid());TEST_ASSERT_FALSE(AutomationCommandArgument::makeFloat(INFINITY).isValid());
    TEST_ASSERT_TRUE(AutomationCommandArgument::makePercentage(0).getPercentage(p));TEST_ASSERT_TRUE(AutomationCommandArgument::makePercentage(100).isValid());
    TEST_ASSERT_FALSE(AutomationCommandArgument::makePercentage(-1).isValid());TEST_ASSERT_FALSE(AutomationCommandArgument::makePercentage(101).isValid());
    TEST_ASSERT_TRUE(AutomationCommandArgument::makeDuration(0).getDuration(u));TEST_ASSERT_EQUAL_UINT32(0,u);
    empty=AutomationCommandArgument::makeEnum(3);empty.clear();TEST_ASSERT_FALSE(empty.isValid());empty=AutomationCommandArgument::makeIdentifier(0);empty.invalidate();TEST_ASSERT_FALSE(empty.isValid());
}

void testActionMappings()
{
    TEST_ASSERT_FALSE(AutomationCommand{}.isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::OUT,1,CommandOperation::ON).isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::OUT,1,CommandOperation::OFF).isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::OUT,1,CommandOperation::TOGGLE).isValid());
    auto pulse=makeAction(CommandDomain::OUT,1,CommandOperation::PULSE);TEST_ASSERT_FALSE(pulse.isValid());pulse.setDuration(1);TEST_ASSERT_TRUE(pulse.isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::IR,1,CommandOperation::SEND).isValid());TEST_ASSERT_TRUE(makeAction(CommandDomain::RF,1,CommandOperation::SEND).isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::SMS,1,CommandOperation::SEND).isValid());TEST_ASSERT_TRUE(makeAction(CommandDomain::CALL,1,CommandOperation::START).isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::CALL,0,CommandOperation::STOP).isValid());TEST_ASSERT_TRUE(makeAction(CommandDomain::SCN,3,CommandOperation::RUN).isValid());TEST_ASSERT_TRUE(makeAction(CommandDomain::NODE,1,CommandOperation::PING).isValid());
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AutomationCommandValidationResult::DANGEROUS_OPERATION_NOT_PERSISTABLE),static_cast<uint8_t>(makeAction(CommandDomain::NODE,1,CommandOperation::REBOOT).validate()));
    TEST_ASSERT_FALSE(makeAction(CommandDomain::SYS,0,CommandOperation::REBOOT).isValid());TEST_ASSERT_FALSE(makeAction(CommandDomain::STORE,0,CommandOperation::CLEAR).isValid());TEST_ASSERT_FALSE(makeAction(CommandDomain::CFG,0,CommandOperation::RESET).isValid());TEST_ASSERT_FALSE(makeAction(CommandDomain::USER,1,CommandOperation::DISABLE).isValid());
    TEST_ASSERT_FALSE(makeAction(CommandDomain::IR,1,CommandOperation::ON).isValid());
    auto invalidDuration=makeAction(CommandDomain::IR,1,CommandOperation::SEND);invalidDuration.setDuration(1);TEST_ASSERT_FALSE(invalidDuration.isValid());
    auto extra=makeAction(CommandDomain::OUT,1,CommandOperation::ON);TEST_ASSERT_TRUE(extra.addArgument(AutomationCommandArgument::makeInteger(1)));TEST_ASSERT_FALSE(extra.isValid());
    auto badIndex=makeAction(CommandDomain::OUT,0,CommandOperation::ON);badIndex.hasDomainIndex=true;TEST_ASSERT_FALSE(badIndex.isValid());
    auto explicitZero=makeAction(CommandDomain::OUT,1,CommandOperation::ON);explicitZero.setDuration(0);TEST_ASSERT_TRUE(explicitZero.hasDurationValue);explicitZero.clearDuration();TEST_ASSERT_FALSE(explicitZero.hasDurationValue);
}

void testDomainIndexPolicy()
{
    auto pulse=makeAction(CommandDomain::OUT,1,CommandOperation::PULSE);pulse.setDuration(1);
    TEST_ASSERT_TRUE(makeAction(CommandDomain::OUT,1,CommandOperation::ON).isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::OUT,1,CommandOperation::OFF).isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::OUT,1,CommandOperation::TOGGLE).isValid());
    TEST_ASSERT_TRUE(pulse.isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::IR,1,CommandOperation::SEND).isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::RF,1,CommandOperation::SEND).isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::NODE,1,CommandOperation::PING).isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::NODE,1,CommandOperation::SYNC).isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::NODE,1,CommandOperation::DISCOVER).isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::SCN,1,CommandOperation::RUN).isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::SCN,1,CommandOperation::STOP).isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::SMS,1,CommandOperation::SEND).isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::CALL,1,CommandOperation::START).isValid());
    TEST_ASSERT_TRUE(makeAction(CommandDomain::CALL,0,CommandOperation::STOP).isValid());

    const AutomationCommandValidationResult expected=AutomationCommandValidationResult::INVALID_DOMAIN_INDEX;
    TEST_ASSERT_TRUE(makeAction(CommandDomain::OUT,0,CommandOperation::ON).validate()==expected);
    TEST_ASSERT_TRUE(makeAction(CommandDomain::IR,0,CommandOperation::SEND).validate()==expected);
    TEST_ASSERT_TRUE(makeAction(CommandDomain::RF,0,CommandOperation::SEND).validate()==expected);
    TEST_ASSERT_TRUE(makeAction(CommandDomain::NODE,0,CommandOperation::PING).validate()==expected);
    TEST_ASSERT_TRUE(makeAction(CommandDomain::SCN,0,CommandOperation::RUN).validate()==expected);
    TEST_ASSERT_TRUE(makeAction(CommandDomain::SMS,0,CommandOperation::SEND).validate()==expected);
    TEST_ASSERT_TRUE(makeAction(CommandDomain::CALL,0,CommandOperation::START).validate()==expected);
    TEST_ASSERT_TRUE(makeAction(CommandDomain::CALL,1,CommandOperation::STOP).validate()==expected);
}

void testRuleBranchIndependence()
{
    Rule rule=makeRule(1,"Branches");
    RuleActionStep elseStep;elseStep.stepIndex=1;elseStep.branch=RuleBranch::ELSE_BRANCH;
    elseStep.command=makeAction(CommandDomain::OUT,2,CommandOperation::OFF);elseStep.delayBeforeMs=20;
    TEST_ASSERT_TRUE(rule.addActionStep(elseStep)==AutomationModelResult::SUCCESS);

    RuleActionStep thenStep;thenStep.stepIndex=1;thenStep.branch=RuleBranch::THEN_BRANCH;
    thenStep.command=makeAction(CommandDomain::OUT,1,CommandOperation::ON);thenStep.delayBeforeMs=10;
    TEST_ASSERT_TRUE(rule.updateActionStep(thenStep)==AutomationModelResult::NOT_FOUND);
    TEST_ASSERT_TRUE(rule.addActionStep(thenStep)==AutomationModelResult::SUCCESS);
    TEST_ASSERT_TRUE(rule.isValid());

    RuleActionStep thenUpdate=thenStep;thenUpdate.delayBeforeMs=100;
    TEST_ASSERT_TRUE(rule.updateActionStep(thenUpdate)==AutomationModelResult::SUCCESS);
    TEST_ASSERT_EQUAL_UINT32(100,rule.findActionStep(RuleBranch::THEN_BRANCH,1)->delayBeforeMs);
    TEST_ASSERT_EQUAL_UINT32(20,rule.findActionStep(RuleBranch::ELSE_BRANCH,1)->delayBeforeMs);

    RuleActionStep elseUpdate=elseStep;elseUpdate.delayBeforeMs=200;
    TEST_ASSERT_TRUE(rule.updateActionStep(elseUpdate)==AutomationModelResult::SUCCESS);
    TEST_ASSERT_EQUAL_UINT32(100,rule.findActionStep(RuleBranch::THEN_BRANCH,1)->delayBeforeMs);
    TEST_ASSERT_EQUAL_UINT32(200,rule.findActionStep(RuleBranch::ELSE_BRANCH,1)->delayBeforeMs);

    RuleActionStep invalidBranch=thenStep;invalidBranch.branch=RuleBranch::NONE;
    TEST_ASSERT_TRUE(rule.updateActionStep(invalidBranch)==AutomationModelResult::BRANCH_MISMATCH);
}

void testStepsAndModels()
{
    SceneStep sceneStep;sceneStep.stepIndex=2;sceneStep.command=makeAction(CommandDomain::OUT,1,CommandOperation::ON);sceneStep.delayAfterMs=20;TEST_ASSERT_TRUE(sceneStep.isValid());
    auto previous=sceneStep.command;TEST_ASSERT_FALSE(sceneStep.setCommand(AutomationCommand{}));TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(previous.operation),static_cast<uint16_t>(sceneStep.command.operation));
    RuleActionStep action;action.stepIndex=2;action.branch=RuleBranch::THEN_BRANCH;action.command=makeAction(CommandDomain::SMS,1,CommandOperation::SEND);action.delayBeforeMs=30;TEST_ASSERT_TRUE(action.isValid());
    ScheduledCommand scheduled;scheduled.scheduleId=1;scheduled.commandIndex=1;scheduled.command=makeAction(CommandDomain::SCN,3,CommandOperation::RUN);scheduled.mode=ScheduleMode::INTERVAL;scheduled.intervalMs=1000;TEST_ASSERT_TRUE(scheduled.isValid());
    Scene scene=makeScene(1,"Arrival");TEST_ASSERT_TRUE(scene.addStep(sceneStep)==AutomationModelResult::SUCCESS);SceneStep second=sceneStep;second.stepIndex=3;TEST_ASSERT_TRUE(scene.addStep(second)==AutomationModelResult::SUCCESS);TEST_ASSERT_TRUE(scene.removeStep(2)==AutomationModelResult::SUCCESS);TEST_ASSERT_EQUAL_UINT16(3,scene.getStepAt(0)->stepIndex);
    Rule rule=makeRule(1,"Notify");TEST_ASSERT_TRUE(rule.addActionStep(action)==AutomationModelResult::SUCCESS);TEST_ASSERT_TRUE(rule.removeActionStep(RuleBranch::THEN_BRANCH,2)==AutomationModelResult::SUCCESS);
    Schedule schedule=makeSchedule(1,"Daily");TEST_ASSERT_TRUE(schedule.addCommand(scheduled)==AutomationModelResult::SUCCESS);TEST_ASSERT_TRUE(schedule.removeCommand(1)==AutomationModelResult::SUCCESS);
}

void testManagers()
{
    SceneManager scenes;RuleManager rules;ScheduleManager schedules;
    TEST_ASSERT_EQUAL_UINT32(16,scenes.capacity());TEST_ASSERT_EQUAL_UINT32(16,rules.capacity());TEST_ASSERT_EQUAL_UINT32(16,schedules.capacity());
    Scene scene=makeScene(1,"Arrival");TEST_ASSERT_TRUE(scenes.add(scene)==AutomationModelResult::SUCCESS);TEST_ASSERT_TRUE(scenes.add(scene)==AutomationModelResult::DUPLICATE_ID);TEST_ASSERT_NOT_NULL(scenes.findById(1));TEST_ASSERT_TRUE(scenes.remove(1)==AutomationModelResult::SUCCESS);
    Rule rule=makeRule(1,"Notify");TEST_ASSERT_TRUE(rules.add(rule)==AutomationModelResult::SUCCESS);TEST_ASSERT_TRUE(rules.update(rule)==AutomationModelResult::SUCCESS);TEST_ASSERT_TRUE(rules.remove(1)==AutomationModelResult::SUCCESS);
    Schedule schedule=makeSchedule(1,"Daily");TEST_ASSERT_TRUE(schedules.add(schedule)==AutomationModelResult::SUCCESS);TEST_ASSERT_TRUE(schedules.update(schedule)==AutomationModelResult::SUCCESS);TEST_ASSERT_TRUE(schedules.remove(1)==AutomationModelResult::SUCCESS);
}

static_assert(sizeof(AutomationCommand)==36U,"Layout حافظه AutomationCommand نباید تغییر کند");
static_assert(std::is_same<decltype(static_cast<const Scene&>(*(Scene*)nullptr).findStep(1)),const SceneStep*>::value,"Scene read API");
static_assert(std::is_same<decltype(static_cast<const Rule&>(*(Rule*)nullptr).findActionStep(RuleBranch::THEN_BRANCH,1)),const RuleActionStep*>::value,"Rule read API");
static_assert(std::is_same<decltype(static_cast<const Schedule&>(*(Schedule*)nullptr).findCommand(1)),const ScheduledCommand*>::value,"Schedule read API");

void setup(){UNITY_BEGIN();RUN_TEST(testArguments);RUN_TEST(testActionMappings);RUN_TEST(testDomainIndexPolicy);RUN_TEST(testRuleBranchIndependence);RUN_TEST(testStepsAndModels);RUN_TEST(testManagers);UNITY_END();}
void loop(){}
