#include <ConfirmTokenIssuer.h>
#include <CommandExecutor.h>
#include <CompositeCommandExecutionGate.h>
#include <RuntimeConfirmTokenProvider.h>
#include <Arduino.h>
#include <unity.h>

namespace
{
Command makeCommand(CommandId commandId=10U, RequestId requestId=20U, ConfirmToken token=0U)
{
    Command command;
    command.context.commandId=commandId;
    command.context.request.requestId=requestId;
    command.context.request.source=static_cast<CommandSource>(5U);
    command.context.risk=CommandRisk::DANGEROUS;
    command.context.confirmToken=token;
    command.domain=CommandDomain::NODE;
    command.domainIndex=1U;
    command.hasDomainIndex=true;
    command.operation=CommandOperation::REBOOT;
    return command;
}

ConfirmTokenRecord makeRecord(
    ConfirmToken value=100U, CommandId commandId=10U, RequestId requestId=20U,
    uint32_t issuedAtMs=50U, uint32_t ttlMs=100U
)
{
    ConfirmTokenRecord token;
    token.id=1U;token.value=value;token.commandId=commandId;token.requestId=requestId;
    token.issuedAtMs=issuedAtMs;token.ttlMs=ttlMs;return token;
}

class FakeGenerator final : public ConfirmTokenGenerator
{
public:
    bool succeeds=true;ConfirmToken value=1234U;uint8_t calls=0U;
    bool generate(ConfirmToken& output) override
    {++calls;if(!succeeds)return false;output=value;return true;}
};

class Mode final : public SystemModeProvider{public:SystemMode getMode()const override{return SystemMode::NORMAL;}SystemModeCheckResult check(const Command&,uint32_t)const override{return SystemModeCheckResult::ALLOWED;}};
class Auth final : public AuthorizationPolicy{public:AuthorizationCheckResult check(const Command&,uint32_t)const override{return AuthorizationCheckResult::ALLOWED;}};
class Safety final : public SafetyPolicy{public:SafetyCheckResult check(const Command&,uint32_t)const override{return SafetyCheckResult::ALLOWED;}};
class Handler final : public CommandHandler
{
public:
    CommandDispatchResult result=CommandDispatchResult::SUCCESS;mutable uint8_t calls=0U;
    bool supports(const Command&)const override{return true;}
    CommandDispatchResult handle(const Command& command,CommandResult& output)const override
    {
        ++calls;if(result!=CommandDispatchResult::SUCCESS)return result;
        output.commandId=command.context.commandId;output.requestId=command.context.request.requestId;
        output.transitionTo(ExecutionStatus::VALIDATING,1U);output.transitionTo(ExecutionStatus::ACCEPTED,2U);
        output.transitionTo(ExecutionStatus::EXECUTING,3U);output.transitionTo(ExecutionStatus::SUCCESS,4U);return result;
    }
};

void drive(CommandExecutor& executor,uint32_t nowMs){for(uint8_t index=0U;index<5U;++index)executor.update(nowMs);}

void assertCheck(ConfirmTokenCheckResult expected, ConfirmTokenCheckResult actual)
{TEST_ASSERT_EQUAL_UINT8((uint8_t)expected,(uint8_t)actual);}
}

void setUp() {}
void tearDown() {}

void test_record_structural_validity_and_expiration()
{
    ConfirmTokenRecord token;TEST_ASSERT_FALSE(token.isValid());
    token=makeRecord();TEST_ASSERT_TRUE(token.isValid());token.consumed=true;TEST_ASSERT_TRUE(token.isValid());
    token=makeRecord(0U);TEST_ASSERT_FALSE(token.isValid());token=makeRecord();token.commandId=0U;TEST_ASSERT_FALSE(token.isValid());
    token=makeRecord();token.requestId=0U;TEST_ASSERT_FALSE(token.isValid());token=makeRecord();token.ttlMs=0U;TEST_ASSERT_FALSE(token.isValid());
    token=makeRecord(100U,10U,20U,0xFFFFFFF0UL,32U);TEST_ASSERT_FALSE(token.isExpired(15U));TEST_ASSERT_TRUE(token.isExpired(16U));
}

void test_store_add_find_remove_clear_and_atomic_failures()
{
    ConfirmTokenStore store;TEST_ASSERT_EQUAL_UINT32(0U,store.size());TEST_ASSERT_EQUAL_UINT32(CONFIRM_TOKEN_CAPACITY,store.capacity());
    ConfirmTokenRecord invalid;TEST_ASSERT_EQUAL_UINT8((uint8_t)ConfirmTokenStoreResult::INVALID_TOKEN,(uint8_t)store.add(invalid));
    ConfirmTokenRecord first=makeRecord(100U);ConfirmTokenRecord second=makeRecord(200U,11U,21U);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)ConfirmTokenStoreResult::SUCCESS,(uint8_t)store.add(first));
    TEST_ASSERT_NOT_NULL(store.findByValue(100U));TEST_ASSERT_NULL(store.findByValue(999U));
    TEST_ASSERT_EQUAL_UINT8((uint8_t)ConfirmTokenStoreResult::DUPLICATE_VALUE,(uint8_t)store.add(first));TEST_ASSERT_EQUAL_UINT32(1U,store.size());
    store.add(second);store.remove(100U);TEST_ASSERT_EQUAL_UINT32(200U,store.findByValue(200U)->value);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)ConfirmTokenStoreResult::TOKEN_NOT_FOUND,(uint8_t)store.remove(999U));TEST_ASSERT_EQUAL_UINT32(1U,store.size());
    store.clear();TEST_ASSERT_EQUAL_UINT32(0U,store.size());
}

void test_store_full_is_atomic()
{
    ConfirmTokenStore store;
    for(size_t index=0U;index<CONFIRM_TOKEN_CAPACITY;++index)TEST_ASSERT_EQUAL_UINT8(0U,(uint8_t)store.add(makeRecord((ConfirmToken)(index+1U),(CommandId)(index+1U),(RequestId)(index+1U))));
    TEST_ASSERT_EQUAL_UINT8((uint8_t)ConfirmTokenStoreResult::STORE_FULL,(uint8_t)store.add(makeRecord(99U,99U,99U)));
    TEST_ASSERT_EQUAL_UINT32(CONFIRM_TOKEN_CAPACITY,store.size());
}

void test_issue_success_and_failures_are_atomic()
{
    ConfirmTokenStore store;FakeGenerator generator;ConfirmTokenIssuer issuer(store,generator,1000U);Command command=makeCommand();ConfirmToken output=77U;
    TEST_ASSERT_EQUAL_UINT8((uint8_t)ConfirmTokenIssueResult::SUCCESS,(uint8_t)issuer.issue(command,50U,output));TEST_ASSERT_EQUAL_UINT32(1234U,output);TEST_ASSERT_EQUAL_UINT8(1U,generator.calls);
    const ConfirmTokenRecord* token=store.findByValue(output);TEST_ASSERT_NOT_NULL(token);TEST_ASSERT_EQUAL_UINT32(10U,token->commandId);TEST_ASSERT_EQUAL_UINT32(20U,token->requestId);TEST_ASSERT_EQUAL_UINT32(50U,token->issuedAtMs);TEST_ASSERT_EQUAL_UINT32(1000U,token->ttlMs);
    output=88U;TEST_ASSERT_EQUAL_UINT8((uint8_t)ConfirmTokenIssueResult::DUPLICATE_VALUE,(uint8_t)issuer.issue(command,60U,output));TEST_ASSERT_EQUAL_UINT32(88U,output);TEST_ASSERT_EQUAL_UINT32(1U,store.size());
    ConfirmTokenStore empty;FakeGenerator failed;failed.succeeds=false;ConfirmTokenIssuer failedIssuer(empty,failed,100U);TEST_ASSERT_EQUAL_UINT8((uint8_t)ConfirmTokenIssueResult::GENERATION_FAILED,(uint8_t)failedIssuer.issue(command,1U,output));TEST_ASSERT_EQUAL_UINT32(0U,empty.size());
    FakeGenerator zero;zero.value=0U;ConfirmTokenIssuer zeroIssuer(empty,zero,100U);TEST_ASSERT_EQUAL_UINT8((uint8_t)ConfirmTokenIssueResult::INVALID_GENERATED_VALUE,(uint8_t)zeroIssuer.issue(command,1U,output));
    ConfirmTokenIssuer badTtl(empty,zero,0U);TEST_ASSERT_EQUAL_UINT8((uint8_t)ConfirmTokenIssueResult::INVALID_TTL,(uint8_t)badTtl.issue(command,1U,output));TEST_ASSERT_EQUAL_UINT8(0U,zero.calls-1U);
}

void test_provider_validation_results_and_read_only_check()
{
    ConfirmTokenStore store;RuntimeConfirmTokenProvider provider(store);Command command=makeCommand();
    command.context.risk=CommandRisk::SAFE;assertCheck(ConfirmTokenCheckResult::NOT_REQUIRED,provider.check(command,1U));
    command=makeCommand();assertCheck(ConfirmTokenCheckResult::REQUIRED,provider.check(command,1U));
    command.context.confirmToken=999U;assertCheck(ConfirmTokenCheckResult::INVALID_TOKEN,provider.check(command,1U));
    store.add(makeRecord());command=makeCommand(10U,20U,100U);assertCheck(ConfirmTokenCheckResult::VALID,provider.check(command,149U));assertCheck(ConfirmTokenCheckResult::VALID,provider.check(command,149U));
    assertCheck(ConfirmTokenCheckResult::EXPIRED_TOKEN,provider.check(command,150U));
    command=makeCommand(11U,20U,100U);assertCheck(ConfirmTokenCheckResult::TOKEN_COMMAND_MISMATCH,provider.check(command,100U));
    command=makeCommand(10U,21U,100U);assertCheck(ConfirmTokenCheckResult::TOKEN_REQUEST_MISMATCH,provider.check(command,100U));
}

void test_consume_exactly_once_and_failure_does_not_mutate()
{
    ConfirmTokenStore store;store.add(makeRecord(100U));store.add(makeRecord(200U,11U,21U));RuntimeConfirmTokenProvider provider(store);
    Command wrong=makeCommand(99U,20U,100U);assertCheck(ConfirmTokenCheckResult::TOKEN_COMMAND_MISMATCH,provider.consume(wrong,100U));TEST_ASSERT_FALSE(store.findByValue(100U)->consumed);
    Command command=makeCommand(10U,20U,100U);assertCheck(ConfirmTokenCheckResult::VALID,provider.consume(command,100U));TEST_ASSERT_TRUE(store.findByValue(100U)->consumed);TEST_ASSERT_FALSE(store.findByValue(200U)->consumed);
    assertCheck(ConfirmTokenCheckResult::TOKEN_ALREADY_USED,provider.consume(command,101U));assertCheck(ConfirmTokenCheckResult::TOKEN_ALREADY_USED,provider.check(command,101U));TEST_ASSERT_EQUAL_UINT32(2U,store.size());
}

void test_composite_executor_integration_consumes_only_after_success()
{
    ConfirmTokenStore store;FakeGenerator generator;ConfirmTokenIssuer issuer(store,generator,1000U);RuntimeConfirmTokenProvider provider(store);
    Mode mode;Auth auth;Safety safety;CompositeCommandExecutionGate gate(mode,auth,safety,provider);
    SceneExecutionQueue queue;CommandValidator validator;Handler handler;CommandDispatcher dispatcher;dispatcher.registerHandler(handler);
    CommandExecutor executor(queue,validator,gate,gate,dispatcher);Command command=makeCommand();ConfirmToken value=0U;
    TEST_ASSERT_EQUAL_UINT8(0U,(uint8_t)issuer.issue(command,10U,value));command.context.confirmToken=value;queue.enqueue(command);drive(executor,20U);
    TEST_ASSERT_EQUAL_UINT8(1U,handler.calls);TEST_ASSERT_TRUE(store.findByValue(value)->consumed);
    executor.reset();queue.enqueue(command);drive(executor,21U);TEST_ASSERT_EQUAL_UINT8(1U,handler.calls);
    ConfirmTokenStore secondStore;FakeGenerator secondGenerator;secondGenerator.value=4321U;ConfirmTokenIssuer secondIssuer(secondStore,secondGenerator,1000U);RuntimeConfirmTokenProvider secondProvider(secondStore);
    CompositeCommandExecutionGate secondGate(mode,auth,safety,secondProvider);SceneExecutionQueue secondQueue;Handler rejecting;rejecting.result=CommandDispatchResult::HANDLER_REJECTED;CommandDispatcher secondDispatcher;secondDispatcher.registerHandler(rejecting);
    CommandExecutor secondExecutor(secondQueue,validator,secondGate,secondGate,secondDispatcher);Command second=makeCommand(30U,40U);ConfirmToken secondValue=0U;secondIssuer.issue(second,10U,secondValue);second.context.confirmToken=secondValue;secondQueue.enqueue(second);drive(secondExecutor,20U);
    TEST_ASSERT_FALSE(secondStore.findByValue(secondValue)->consumed);
}

void setup()
{
    UNITY_BEGIN();RUN_TEST(test_record_structural_validity_and_expiration);RUN_TEST(test_store_add_find_remove_clear_and_atomic_failures);RUN_TEST(test_store_full_is_atomic);RUN_TEST(test_issue_success_and_failures_are_atomic);RUN_TEST(test_provider_validation_results_and_read_only_check);RUN_TEST(test_consume_exactly_once_and_failure_does_not_mutate);RUN_TEST(test_composite_executor_integration_consumes_only_after_success);UNITY_END();
}
void loop() {}
