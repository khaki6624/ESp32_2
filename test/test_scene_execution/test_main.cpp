#include <SceneExecutor.h>
#include <Arduino.h>
#include <unity.h>
#include <type_traits>

static SceneManager managerValue;
static AutomationCommandFactory factoryValue;
static SceneExecutionQueue queueValue;
static SceneExecutor executorValue(managerValue,factoryValue,queueValue);

static RequestContext makeRequestContext(RequestId id=1,CommandSource source=CommandSource::APP)
{RequestContext value;value.requestId=id;value.source=source;return value;}
static SceneExecutionRequest makeExecutionRequest(SceneId sceneId=1,CommandId firstId=100)
{SceneExecutionRequest value;value.sceneId=sceneId;value.request=makeRequestContext();value.firstCommandId=firstId;return value;}
static AutomationCommand makeAction(CommandDomain domain,uint16_t index,CommandOperation operation)
{AutomationCommand value;value.domain=domain;value.domainIndex=index;value.hasDomainIndex=index!=0U;value.operation=operation;return value;}
static SceneStep makeStep(AutomationStepIndex index,uint32_t delayMs=0)
{SceneStep value;value.stepIndex=index;value.command=makeAction(CommandDomain::OUT,index,CommandOperation::ON);value.delayAfterMs=delayMs;return value;}
static Scene makeScene(SceneId id,size_t stepCount=1)
{Scene value;value.id=id;value.setName("Execution");value.enabled=true;for(size_t i=0;i<stepCount;++i)value.addStep(makeStep(static_cast<AutomationStepIndex>(i+1U)));return value;}
static void resetFixture(){executorValue.reset();queueValue.clear();managerValue.clear();}

void testFactory()
{
    AutomationCommand source=makeAction(CommandDomain::OUT,1,CommandOperation::ON);
    RequestContext request=makeRequestContext(7,CommandSource::SMS);
    Command output;
    TEST_ASSERT_TRUE(factoryValue.create(source,request,9,123,output)==AutomationCommandFactoryResult::SUCCESS);
    TEST_ASSERT_TRUE(output.isValid());TEST_ASSERT_EQUAL_UINT32(9,output.context.commandId);
    TEST_ASSERT_EQUAL_UINT32(7,output.context.request.requestId);TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandSource::SMS),static_cast<uint8_t>(output.context.request.source));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandPriority::NORMAL),static_cast<uint8_t>(output.context.priority));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandRisk::ACTION),static_cast<uint8_t>(output.context.risk));
    TEST_ASSERT_EQUAL_UINT16(1,output.domainIndex);TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(CommandOperation::ON),static_cast<uint16_t>(output.operation));
    TEST_ASSERT_EQUAL_UINT32(0,output.context.confirmToken);TEST_ASSERT_FALSE(output.context.confirmationRequired);
    TEST_ASSERT_EQUAL_CHAR('\0',output.originalText[0]);TEST_ASSERT_TRUE(CommandText::isCanonical(output.originalText,sizeof(output.originalText)));
    source.setDuration(0);TEST_ASSERT_TRUE(factoryValue.create(source,request,10,124,output)==AutomationCommandFactoryResult::SUCCESS);TEST_ASSERT_TRUE(output.hasDuration());TEST_ASSERT_EQUAL_UINT32(0,output.durationMs);
    source.setDuration(50);TEST_ASSERT_TRUE(factoryValue.create(source,request,11,125,output)==AutomationCommandFactoryResult::SUCCESS);TEST_ASSERT_EQUAL_UINT32(50,output.durationMs);
    Command unchanged=output;TEST_ASSERT_TRUE(factoryValue.create(AutomationCommand{},request,12,0,output)==AutomationCommandFactoryResult::INVALID_AUTOMATION_COMMAND);TEST_ASSERT_EQUAL_UINT32(unchanged.context.commandId,output.context.commandId);
    AutomationCommand safe=makeAction(CommandDomain::NODE,1,CommandOperation::PING);TEST_ASSERT_TRUE(factoryValue.create(safe,request,13,0,output)==AutomationCommandFactoryResult::SUCCESS);TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandRisk::SAFE),static_cast<uint8_t>(output.context.risk));
}

void testCommandQueue()
{
    resetFixture();TEST_ASSERT_TRUE(queueValue.isEmpty());TEST_ASSERT_EQUAL_UINT32(8,queueValue.capacity());
    Command command;TEST_ASSERT_TRUE(queueValue.enqueue(command)==SceneExecutionResult::COMMAND_FACTORY_FAILED);
    AutomationCommand source=makeAction(CommandDomain::OUT,1,CommandOperation::ON);RequestContext request=makeRequestContext();
    for(size_t i=0;i<8;++i){TEST_ASSERT_TRUE(factoryValue.create(source,request,static_cast<CommandId>(i+1U),0,command)==AutomationCommandFactoryResult::SUCCESS);TEST_ASSERT_TRUE(queueValue.enqueue(command)==SceneExecutionResult::SUCCESS);}
    TEST_ASSERT_TRUE(queueValue.isFull());const size_t before=queueValue.size();TEST_ASSERT_TRUE(queueValue.enqueue(command)==SceneExecutionResult::COMMAND_QUEUE_FULL);TEST_ASSERT_EQUAL_UINT32(before,queueValue.size());
    TEST_ASSERT_EQUAL_UINT32(1,queueValue.peek()->context.commandId);TEST_ASSERT_TRUE(queueValue.consume()==SceneExecutionResult::SUCCESS);TEST_ASSERT_EQUAL_UINT32(2,queueValue.peek()->context.commandId);
    queueValue.clear();TEST_ASSERT_TRUE(queueValue.consume()==SceneExecutionResult::COMMAND_QUEUE_EMPTY);TEST_ASSERT_NULL(queueValue.peek());
}

void testRequestAndStartPolicies()
{
    resetFixture();SceneExecutionRequest invalid;TEST_ASSERT_FALSE(invalid.isValid());TEST_ASSERT_TRUE(executorValue.enqueue(invalid)==SceneExecutionResult::INVALID_SCENE_ID);
    Scene scene=makeScene(1);scene.enabled=false;TEST_ASSERT_TRUE(managerValue.add(scene)==AutomationModelResult::SUCCESS);TEST_ASSERT_TRUE(executorValue.start(makeExecutionRequest())==SceneExecutionResult::SCENE_DISABLED);
    managerValue.clear();Scene empty=makeScene(1,0);TEST_ASSERT_TRUE(managerValue.add(empty)==AutomationModelResult::SUCCESS);TEST_ASSERT_TRUE(executorValue.start(makeExecutionRequest())==SceneExecutionResult::SCENE_EMPTY);
    managerValue.clear();scene=makeScene(1);TEST_ASSERT_TRUE(managerValue.add(scene)==AutomationModelResult::SUCCESS);
    SceneExecutionRequest request=makeExecutionRequest();TEST_ASSERT_TRUE(executorValue.enqueue(request)==SceneExecutionResult::SUCCESS);TEST_ASSERT_TRUE(executorValue.enqueue(request)==SceneExecutionResult::SUCCESS);TEST_ASSERT_EQUAL_UINT32(2,executorValue.pendingRequestCount());
    TEST_ASSERT_TRUE(executorValue.start(request)==SceneExecutionResult::ALREADY_RUNNING);executorValue.clearRequests();TEST_ASSERT_FALSE(executorValue.hasPendingRequests());
    TEST_ASSERT_TRUE(executorValue.start(request)==SceneExecutionResult::SUCCESS);TEST_ASSERT_TRUE(executorValue.start(request)==SceneExecutionResult::ALREADY_RUNNING);
}

void testExecutionDelayAndCompletion()
{
    resetFixture();Scene scene=makeScene(1,0);SceneStep first=makeStep(1,5);SceneStep second=makeStep(2);scene.addStep(first);scene.addStep(second);TEST_ASSERT_TRUE(managerValue.add(scene)==AutomationModelResult::SUCCESS);
    SceneExecutionRequest request=makeExecutionRequest(1,200);request.request.requestId=77;TEST_ASSERT_TRUE(executorValue.start(request)==SceneExecutionResult::SUCCESS);
    executorValue.update(0xFFFFFFFEU);TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::WAITING_DELAY);TEST_ASSERT_EQUAL_UINT32(200,queueValue.getAt(0)->context.commandId);
    executorValue.update(1U);TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::WAITING_DELAY);
    executorValue.update(3U);TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::COMPLETED);TEST_ASSERT_EQUAL_UINT32(2,queueValue.size());TEST_ASSERT_EQUAL_UINT32(201,queueValue.getAt(1)->context.commandId);TEST_ASSERT_EQUAL_UINT32(77,queueValue.getAt(1)->context.request.requestId);
}

void testBackpressureCancellationAndReset()
{
    resetFixture();Scene scene=makeScene(1);TEST_ASSERT_TRUE(managerValue.add(scene)==AutomationModelResult::SUCCESS);
    AutomationCommand source=makeAction(CommandDomain::OUT,1,CommandOperation::ON);RequestContext context=makeRequestContext();Command command;
    for(CommandId id=1;id<=8;++id){factoryValue.create(source,context,id,0,command);queueValue.enqueue(command);}
    TEST_ASSERT_TRUE(executorValue.start(makeExecutionRequest())==SceneExecutionResult::SUCCESS);executorValue.update(0);TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::WAITING_COMMAND_QUEUE);TEST_ASSERT_EQUAL_UINT16(1,executorValue.getCurrentStepIndex());
    queueValue.consume();executorValue.update(1);TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::COMPLETED);TEST_ASSERT_EQUAL_UINT32(8,queueValue.size());
    executorValue.update(2);TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::IDLE);TEST_ASSERT_TRUE(executorValue.enqueue(makeExecutionRequest())==SceneExecutionResult::SUCCESS);TEST_ASSERT_TRUE(executorValue.enqueue(makeExecutionRequest())==SceneExecutionResult::SUCCESS);executorValue.update(3);TEST_ASSERT_EQUAL_UINT32(1,executorValue.pendingRequestCount());TEST_ASSERT_TRUE(executorValue.cancelCurrent()==SceneExecutionResult::CANCELLED);TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::CANCELLED);TEST_ASSERT_EQUAL_UINT32(1,executorValue.pendingRequestCount());TEST_ASSERT_EQUAL_UINT32(8,queueValue.size());
    executorValue.reset();TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::IDLE);TEST_ASSERT_EQUAL_UINT32(8,queueValue.size());
}

void testFailuresAndNestedScene()
{
    resetFixture();Scene scene=makeScene(1);TEST_ASSERT_TRUE(managerValue.add(scene)==AutomationModelResult::SUCCESS);TEST_ASSERT_TRUE(executorValue.start(makeExecutionRequest())==SceneExecutionResult::SUCCESS);TEST_ASSERT_TRUE(managerValue.remove(1)==AutomationModelResult::SUCCESS);executorValue.update(0);TEST_ASSERT_TRUE(executorValue.getLastResult()==SceneExecutionResult::SCENE_NOT_FOUND);
    resetFixture();Scene nested=makeScene(1,0);SceneStep step;step.stepIndex=1;step.command=makeAction(CommandDomain::SCN,3,CommandOperation::RUN);nested.addStep(step);TEST_ASSERT_TRUE(managerValue.add(nested)==AutomationModelResult::SUCCESS);TEST_ASSERT_TRUE(executorValue.start(makeExecutionRequest())==SceneExecutionResult::SUCCESS);executorValue.update(0);TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandDomain::SCN),static_cast<uint8_t>(queueValue.peek()->domain));TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::COMPLETED);
}

void testCommandIdRangeAtDirectStart()
{
    resetFixture();Scene scene=makeScene(1,1);TEST_ASSERT_TRUE(managerValue.add(scene)==AutomationModelResult::SUCCESS);
    TEST_ASSERT_TRUE(executorValue.start(makeExecutionRequest(1,UINT32_MAX))==SceneExecutionResult::SUCCESS);executorValue.update(0);
    TEST_ASSERT_EQUAL_UINT32(1,queueValue.size());TEST_ASSERT_EQUAL_UINT32(UINT32_MAX,queueValue.peek()->context.commandId);TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::COMPLETED);

    resetFixture();scene=makeScene(1,2);TEST_ASSERT_TRUE(managerValue.add(scene)==AutomationModelResult::SUCCESS);const size_t before=queueValue.size();
    TEST_ASSERT_TRUE(executorValue.start(makeExecutionRequest(1,UINT32_MAX))==SceneExecutionResult::INVALID_COMMAND_ID);
    TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::IDLE);TEST_ASSERT_EQUAL_UINT16(INVALID_SCENE_ID,executorValue.getCurrentSceneId());TEST_ASSERT_EQUAL_UINT32(before,queueValue.size());TEST_ASSERT_TRUE(queueValue.isEmpty());

    TEST_ASSERT_TRUE(executorValue.start(makeExecutionRequest(1,UINT32_MAX-1U))==SceneExecutionResult::SUCCESS);executorValue.update(1);executorValue.update(2);
    TEST_ASSERT_EQUAL_UINT32(2,queueValue.size());TEST_ASSERT_EQUAL_UINT32(UINT32_MAX-1U,queueValue.getAt(0)->context.commandId);TEST_ASSERT_EQUAL_UINT32(UINT32_MAX,queueValue.getAt(1)->context.commandId);TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::COMPLETED);
}

void testSixteenStepCommandIdRange()
{
    resetFixture();Scene scene=makeScene(1,16);TEST_ASSERT_TRUE(managerValue.add(scene)==AutomationModelResult::SUCCESS);
    TEST_ASSERT_TRUE(executorValue.start(makeExecutionRequest(1,UINT32_MAX-15U))==SceneExecutionResult::SUCCESS);
    for(size_t i=0;i<16U;++i){executorValue.update(static_cast<uint32_t>(i));TEST_ASSERT_FALSE(queueValue.isEmpty());TEST_ASSERT_EQUAL_UINT32(UINT32_MAX-15U+i,queueValue.peek()->context.commandId);TEST_ASSERT_TRUE(queueValue.consume()==SceneExecutionResult::SUCCESS);}
    TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::COMPLETED);TEST_ASSERT_TRUE(queueValue.isEmpty());

    resetFixture();scene=makeScene(1,16);TEST_ASSERT_TRUE(managerValue.add(scene)==AutomationModelResult::SUCCESS);const size_t before=queueValue.size();
    TEST_ASSERT_TRUE(executorValue.start(makeExecutionRequest(1,UINT32_MAX-14U))==SceneExecutionResult::INVALID_COMMAND_ID);TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::IDLE);TEST_ASSERT_EQUAL_UINT32(before,queueValue.size());
}

void testPendingInvalidCommandIdRange()
{
    resetFixture();Scene invalidRangeScene=makeScene(1,2);Scene validScene=makeScene(2,1);TEST_ASSERT_TRUE(managerValue.add(invalidRangeScene)==AutomationModelResult::SUCCESS);TEST_ASSERT_TRUE(managerValue.add(validScene)==AutomationModelResult::SUCCESS);
    TEST_ASSERT_TRUE(executorValue.enqueue(makeExecutionRequest(1,UINT32_MAX))==SceneExecutionResult::SUCCESS);TEST_ASSERT_TRUE(executorValue.enqueue(makeExecutionRequest(2,42))==SceneExecutionResult::SUCCESS);const size_t before=queueValue.size();
    executorValue.update(0);TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::FAILED);TEST_ASSERT_TRUE(executorValue.getLastResult()==SceneExecutionResult::INVALID_COMMAND_ID);TEST_ASSERT_EQUAL_UINT32(before,queueValue.size());TEST_ASSERT_EQUAL_UINT32(1,executorValue.pendingRequestCount());
    executorValue.update(1);TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::READY_STEP);TEST_ASSERT_EQUAL_UINT16(2,executorValue.getCurrentSceneId());TEST_ASSERT_TRUE(queueValue.isEmpty());
    executorValue.update(2);TEST_ASSERT_TRUE(executorValue.getState()==SceneExecutionState::COMPLETED);TEST_ASSERT_EQUAL_UINT32(42,queueValue.peek()->context.commandId);
}

static_assert(std::is_same<decltype(static_cast<const SceneExecutionQueue&>(queueValue).peek()),const Command*>::value,"Queue API باید const-only باشد");
void setup(){UNITY_BEGIN();RUN_TEST(testFactory);RUN_TEST(testCommandQueue);RUN_TEST(testRequestAndStartPolicies);RUN_TEST(testExecutionDelayAndCompletion);RUN_TEST(testBackpressureCancellationAndReset);RUN_TEST(testFailuresAndNestedScene);RUN_TEST(testCommandIdRangeAtDirectStart);RUN_TEST(testSixteenStepCommandIdRange);RUN_TEST(testPendingInvalidCommandIdRange);UNITY_END();}
void loop(){}
