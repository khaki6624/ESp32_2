#include <CommunicationRuntime.h>
#include <string.h>
#include <type_traits>
#include <unity.h>

namespace
{
constexpr size_t SEQUENCE_CAPACITY=8U;
void assertResult(CommunicationResult e,CommunicationResult a){TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(e),static_cast<uint8_t>(a));}
CommunicationAddress address(const char* text){CommunicationAddress a;TEST_ASSERT_TRUE(a.set(text));return a;}

class FakeBackend final:public CommunicationBackend
{
public:
 CommunicationBackendId id;CommunicationType type;bool ready=true;
 CommunicationBackendResult beginResult=CommunicationBackendResult::SUCCESS;
 CommunicationBackendResult startResult=CommunicationBackendResult::ACCEPTED;
 size_t startLength=0U;CommunicationBackendResult cancelResult=CommunicationBackendResult::CANCELLED;
 CommunicationBackendResult updates[SEQUENCE_CAPACITY]={};size_t updateLengths[SEQUENCE_CAPACITY]={};size_t updateCount=0U,updateIndex=0U;
 CommunicationBackendResult pollResult=CommunicationBackendResult::NO_MESSAGE;char pollSource[COMMUNICATION_MAX_ADDRESS_LENGTH]={};
 uint8_t pollData[COMMUNICATION_MAX_PAYLOAD_LENGTH]={};size_t pollLength=0U;
 size_t beginCalls=0U,startCalls=0U,updateCalls=0U,cancelCalls=0U,pollCalls=0U;CommunicationMessage lastSent{};CommunicationMessageId lastPollId=0U;
 FakeBackend(CommunicationBackendId backendId,CommunicationType communicationType):id(backendId),type(communicationType){strcpy(pollSource,"source/default");}
 CommunicationBackendId backendId()const override{return id;} CommunicationType communicationType()const override{return type;}
 CommunicationBackendResult begin()override{++beginCalls;return beginResult;} bool isReady()const override{return ready;}
 CommunicationBackendResult startSend(const CommunicationMessage& m,size_t& n)override{++startCalls;lastSent=m;n=startLength;return startResult;}
 CommunicationBackendResult updateSend(size_t& n)override{++updateCalls;if(updateIndex>=updateCount)return CommunicationBackendResult::FAILED;n=updateLengths[updateIndex];return updates[updateIndex++];}
 CommunicationBackendResult cancelSend()override{++cancelCalls;return cancelResult;}
 CommunicationBackendResult pollReceive(CommunicationMessageId mid,CommunicationWritePayload& buffer,CommunicationAddress& source,size_t& length)override
 {++pollCalls;lastPollId=mid;length=pollLength;if(pollResult==CommunicationBackendResult::SUCCESS&&pollLength<=buffer.capacity()){memcpy(buffer.data(),pollData,pollLength);source.set(pollSource);}return pollResult;}
 void addUpdate(CommunicationBackendResult r,size_t n){updates[updateCount]=r;updateLengths[updateCount++]=n;}
 void setReceive(const char* source,const uint8_t* data,size_t length){memset(pollSource,0,sizeof(pollSource));memcpy(pollSource,source,strlen(source));memcpy(pollData,data,length);pollLength=length;pollResult=CommunicationBackendResult::SUCCESS;}
};
class FakeSink final:public InboundMessageSink
{
public:InboundMessageSinkResult configured=InboundMessageSinkResult::ACCEPTED;size_t calls=0U,length=0U;CommunicationMessageId id=0U;CommunicationBackendId backend=0U;CommunicationType type=CommunicationType::COUNT;char source[COMMUNICATION_MAX_ADDRESS_LENGTH]={};uint8_t data[COMMUNICATION_MAX_PAYLOAD_LENGTH]={};
 InboundMessageSinkResult onMessage(const CommunicationMessage& m)override{++calls;id=m.messageId();backend=m.backendId();type=m.communicationType();length=m.receivedLength();strcpy(source,m.address().c_str());memcpy(data,m.inboundPayload().data(),length);return configured;}
};
}

void test_address_payload_and_message_contracts()
{
 CommunicationAddress a;TEST_ASSERT_FALSE(a.set(nullptr));TEST_ASSERT_FALSE(a.set(""));TEST_ASSERT_TRUE(a.set("+98912/device:1@test.local-A_B"));
 const char* bad[]={"bad value","bad\\value","bad=value","bad|value","bad\nvalue"};for(size_t i=0U;i<5U;++i){TEST_ASSERT_FALSE(a.set(bad[i]));TEST_ASSERT_EQUAL_STRING("+98912/device:1@test.local-A_B",a.c_str());}
 char maximum[COMMUNICATION_MAX_ADDRESS_LENGTH]={};memset(maximum,'a',sizeof(maximum)-1U);TEST_ASSERT_TRUE(a.set(maximum));char noNull[COMMUNICATION_MAX_ADDRESS_LENGTH];memset(noNull,'a',sizeof(noNull));TEST_ASSERT_FALSE(a.set(noNull));
 CommunicationAddress same;same.set(maximum);TEST_ASSERT_TRUE(a.equals(same));TEST_ASSERT_FALSE(a.equals(address("A")));TEST_ASSERT_FALSE(address("A").equals(address("a")));
 uint8_t binary[]={0x41U,0x00U,0xD8U,0xAFU,'\n','|'};CommunicationReadPayload read(binary,sizeof(binary));CommunicationWritePayload write(binary,sizeof(binary));
 TEST_ASSERT_TRUE(read.isValid());TEST_ASSERT_TRUE(write.isValid());TEST_ASSERT_EQUAL_PTR(binary,read.data());TEST_ASSERT_EQUAL_UINT32(sizeof(binary),read.length());
 TEST_ASSERT_FALSE(CommunicationReadPayload(nullptr,1U).isValid());TEST_ASSERT_FALSE(CommunicationReadPayload(binary,0U).isValid());TEST_ASSERT_FALSE(CommunicationWritePayload(binary,0U).isValid());
 CommunicationMessage out=CommunicationMessage::outbound(1U,2U,CommunicationType::MQTT,address("device/1"),read);CommunicationMessage in=CommunicationMessage::inbound(2U,2U,CommunicationType::MQTT,address("source/1"),write,sizeof(binary));
 TEST_ASSERT_TRUE(out.isValid());TEST_ASSERT_TRUE(in.isValid());TEST_ASSERT_EQUAL_PTR(binary,out.outboundPayload().data());TEST_ASSERT_FALSE(CommunicationMessage::outbound(0U,2U,CommunicationType::MQTT,address("x"),read).isValid());
 TEST_ASSERT_FALSE(CommunicationMessage::inbound(1U,2U,CommunicationType::MQTT,address("x"),write,0U).isValid());TEST_ASSERT_FALSE(CommunicationMessage::inbound(1U,2U,CommunicationType::MQTT,address("x"),write,sizeof(binary)+1U).isValid());
}

void test_registry_registration_capacity_lock_and_independence()
{
 CommunicationRegistry registry;FakeBackend backends[COMMUNICATION_MAX_BACKENDS+1U]={{1U,CommunicationType::SMS},{2U,CommunicationType::MQTT},{3U,CommunicationType::TCP},{4U,CommunicationType::UDP},{5U,CommunicationType::HTTP},{6U,CommunicationType::LOCAL},{7U,CommunicationType::CUSTOM},{8U,CommunicationType::MODBUS},{9U,CommunicationType::SERIAL}};
 TEST_ASSERT_EQUAL_UINT32(0U,registry.size());for(size_t i=0U;i<COMMUNICATION_MAX_BACKENDS;++i)assertResult(CommunicationResult::SUCCESS,registry.registerBackend(backends[i]));
 TEST_ASSERT_EQUAL_PTR(&backends[2],registry.find(3U));TEST_ASSERT_NULL(registry.find(0U));TEST_ASSERT_NULL(registry.find(99U));TEST_ASSERT_EQUAL_UINT32(0U,backends[0].beginCalls);
 assertResult(CommunicationResult::REGISTRY_FULL,registry.registerBackend(backends[8]));TEST_ASSERT_EQUAL_UINT32(COMMUNICATION_MAX_BACKENDS,registry.size());
 CommunicationRegistry duplicateRegistry;assertResult(CommunicationResult::SUCCESS,duplicateRegistry.registerBackend(backends[0]));assertResult(CommunicationResult::DUPLICATE_BACKEND_ID,duplicateRegistry.registerBackend(backends[0]));
 FakeBackend invalid(0U,CommunicationType::SMS);assertResult(CommunicationResult::INVALID_BACKEND_ID,duplicateRegistry.registerBackend(invalid));duplicateRegistry.lock();assertResult(CommunicationResult::REGISTRY_LOCKED,duplicateRegistry.registerBackend(backends[1]));
}

void test_begin_all_backends_severity_lock_and_busy_rejection()
{
 FakeBackend one(1U,CommunicationType::SMS),two(2U,CommunicationType::MQTT);CommunicationRegistry registry;registry.registerBackend(one);registry.registerBackend(two);FakeSink sink;uint8_t rx[32]={};CommunicationRuntime runtime(registry,sink,CommunicationWritePayload(rx,sizeof(rx)));
 two.beginResult=CommunicationBackendResult::RETRY_LATER;assertResult(CommunicationResult::BACKEND_RETRY_LATER,runtime.begin());TEST_ASSERT_FALSE(runtime.isInitialized());TEST_ASSERT_EQUAL_UINT32(1U,one.beginCalls);TEST_ASSERT_EQUAL_UINT32(1U,two.beginCalls);TEST_ASSERT_TRUE(registry.isLocked());
 two.beginResult=CommunicationBackendResult::SUCCESS;assertResult(CommunicationResult::SUCCESS,runtime.begin());TEST_ASSERT_TRUE(runtime.isInitialized());
 uint8_t data[]={1U,2U};CommunicationMessage m=CommunicationMessage::outbound(10U,1U,CommunicationType::SMS,address("+98912"),CommunicationReadPayload(data,2U));runtime.send(m);size_t calls=one.beginCalls+two.beginCalls;assertResult(CommunicationResult::BUSY,runtime.begin());TEST_ASSERT_EQUAL_UINT32(calls,one.beginCalls+two.beginCalls);TEST_ASSERT_TRUE(runtime.isBackendBusy(1U));
}

void test_send_mapping_length_validation_and_one_tx_per_backend()
{
 FakeBackend one(1U,CommunicationType::SMS),two(2U,CommunicationType::MQTT);CommunicationRegistry registry;registry.registerBackend(one);registry.registerBackend(two);FakeSink sink;uint8_t rx[16]={};CommunicationRuntime runtime(registry,sink,CommunicationWritePayload(rx,sizeof(rx)));uint8_t data[]={0x41U,0U,0x42U};CommunicationMessage sms=CommunicationMessage::outbound(100U,1U,CommunicationType::SMS,address("+98912"),CommunicationReadPayload(data,3U));
 assertResult(CommunicationResult::NOT_INITIALIZED,runtime.send(sms));runtime.begin();one.startLength=1U;assertResult(CommunicationResult::ACCEPTED,runtime.send(sms));TEST_ASSERT_TRUE(runtime.isBackendBusy(1U));TEST_ASSERT_EQUAL_PTR(data,one.lastSent.outboundPayload().data());TEST_ASSERT_EQUAL_UINT32(1U,runtime.transaction(1U)->transferredLength());
 assertResult(CommunicationResult::BUSY,runtime.send(sms));TEST_ASSERT_EQUAL_UINT32(1U,one.startCalls);CommunicationMessage mqtt=CommunicationMessage::outbound(100U,2U,CommunicationType::MQTT,address("topic/a"),CommunicationReadPayload(data,3U));assertResult(CommunicationResult::ACCEPTED,runtime.send(mqtt));TEST_ASSERT_TRUE(runtime.isBackendBusy(2U));
 one.cancelResult=CommunicationBackendResult::SUCCESS;runtime.cancel(1U);one.startResult=CommunicationBackendResult::SUCCESS;one.startLength=3U;assertResult(CommunicationResult::SUCCESS,runtime.send(sms));TEST_ASSERT_EQUAL_UINT32(3U,runtime.transaction(1U)->transferredLength());
 one.startLength=4U;assertResult(CommunicationResult::INTERNAL_ERROR,runtime.send(sms));one.startResult=CommunicationBackendResult::FAILED;one.startLength=1U;assertResult(CommunicationResult::INTERNAL_ERROR,runtime.send(sms));
 const uint8_t expected[]={0x41U,0U,0x42U};TEST_ASSERT_EQUAL_MEMORY(expected,data,3U);CommunicationMessage wrong=CommunicationMessage::outbound(101U,1U,CommunicationType::MQTT,address("x"),CommunicationReadPayload(data,3U));assertResult(CommunicationResult::INVALID_MESSAGE,runtime.send(wrong));
}

void test_tx_update_progress_retry_success_and_fail_closed()
{
 FakeBackend backend(1U,CommunicationType::SMS);CommunicationRegistry registry;registry.registerBackend(backend);FakeSink sink;uint8_t rx[8]={},data[3]={1U,2U,3U};CommunicationRuntime runtime(registry,sink,CommunicationWritePayload(rx,8U));runtime.begin();backend.addUpdate(CommunicationBackendResult::IN_PROGRESS,2U);backend.addUpdate(CommunicationBackendResult::RETRY_LATER,2U);backend.addUpdate(CommunicationBackendResult::SUCCESS,3U);runtime.send(CommunicationMessage::outbound(1U,1U,CommunicationType::SMS,address("x"),CommunicationReadPayload(data,3U)));
 assertResult(CommunicationResult::IN_PROGRESS,runtime.update());assertResult(CommunicationResult::BACKEND_RETRY_LATER,runtime.update());assertResult(CommunicationResult::SUCCESS,runtime.update());TEST_ASSERT_FALSE(runtime.isBackendBusy(1U));TEST_ASSERT_EQUAL_UINT32(3U,runtime.transaction(1U)->transferredLength());
 backend.startResult=CommunicationBackendResult::ACCEPTED;backend.startLength=2U;backend.updateCount=backend.updateIndex=0U;backend.addUpdate(CommunicationBackendResult::IN_PROGRESS,1U);runtime.send(CommunicationMessage::outbound(2U,1U,CommunicationType::SMS,address("x"),CommunicationReadPayload(data,3U)));assertResult(CommunicationResult::INTERNAL_ERROR,runtime.update());TEST_ASSERT_FALSE(runtime.isBackendBusy(1U));
}

void test_rx_round_robin_delivery_validation_and_sink_policy()
{
 FakeBackend one(1U,CommunicationType::SMS),two(2U,CommunicationType::MQTT);CommunicationRegistry registry;registry.registerBackend(one);registry.registerBackend(two);FakeSink sink;uint8_t rx[16]={};CommunicationRuntime runtime(registry,sink,CommunicationWritePayload(rx,sizeof(rx)));runtime.begin();uint8_t bytes[]={0x10U,0U,0xFFU};
 one.pollResult=CommunicationBackendResult::NO_MESSAGE;two.setReceive("device/42",bytes,3U);assertResult(CommunicationResult::NO_MESSAGE,runtime.update());TEST_ASSERT_EQUAL_UINT32(1U,one.pollCalls);assertResult(CommunicationResult::SUCCESS,runtime.update());TEST_ASSERT_EQUAL_UINT32(1U,two.pollCalls);TEST_ASSERT_EQUAL_UINT32(1U,sink.calls);TEST_ASSERT_EQUAL_UINT16(2U,sink.backend);TEST_ASSERT_EQUAL_MEMORY(bytes,sink.data,3U);
 CommunicationMessageId first=sink.id;one.setReceive("sms/source",bytes,3U);assertResult(CommunicationResult::SUCCESS,runtime.update());TEST_ASSERT_TRUE(sink.id>first);
 sink.configured=InboundMessageSinkResult::REJECTED;two.setReceive("device/42",bytes,3U);assertResult(CommunicationResult::SINK_REJECTED,runtime.update());sink.configured=InboundMessageSinkResult::RETRY_LATER;one.setReceive("sms/source",bytes,3U);assertResult(CommunicationResult::SINK_REJECTED,runtime.update());
 two.pollResult=CommunicationBackendResult::SUCCESS;two.pollLength=0U;assertResult(CommunicationResult::INTERNAL_ERROR,runtime.update());
}

void test_cancel_clear_and_multiple_runtime_instances()
{
 FakeBackend b1(1U,CommunicationType::LOCAL),b2(1U,CommunicationType::LOCAL);CommunicationRegistry r1,r2;r1.registerBackend(b1);r2.registerBackend(b2);FakeSink s1,s2;uint8_t x1[8]={},x2[8]={},data[1]={1U};CommunicationRuntime one(r1,s1,CommunicationWritePayload(x1,8U)),two(r2,s2,CommunicationWritePayload(x2,8U));
 assertResult(CommunicationResult::NOT_INITIALIZED,one.cancel(1U));one.begin();two.begin();assertResult(CommunicationResult::INVALID_ARGUMENT,one.cancel(1U));one.send(CommunicationMessage::outbound(1U,1U,CommunicationType::LOCAL,address("local"),CommunicationReadPayload(data,1U)));b1.cancelResult=CommunicationBackendResult::RETRY_LATER;assertResult(CommunicationResult::BACKEND_RETRY_LATER,one.cancel(1U));TEST_ASSERT_TRUE(one.isBackendBusy(1U));b1.cancelResult=CommunicationBackendResult::SUCCESS;assertResult(CommunicationResult::CANCELLED,one.cancel(1U));assertResult(CommunicationResult::SUCCESS,one.clearCompleted(1U));TEST_ASSERT_FALSE(one.transaction(1U)->isValid());
 two.send(CommunicationMessage::outbound(1U,1U,CommunicationType::LOCAL,address("local"),CommunicationReadPayload(data,1U)));TEST_ASSERT_TRUE(two.isBackendBusy(1U));TEST_ASSERT_FALSE(one.isBackendBusy(1U));
}

static_assert(std::is_base_of<CommunicationBackend,FakeBackend>::value,"backend contract");
static_assert(std::is_base_of<InboundMessageSink,FakeSink>::value,"sink contract");
void setup(){UNITY_BEGIN();RUN_TEST(test_address_payload_and_message_contracts);RUN_TEST(test_registry_registration_capacity_lock_and_independence);RUN_TEST(test_begin_all_backends_severity_lock_and_busy_rejection);RUN_TEST(test_send_mapping_length_validation_and_one_tx_per_backend);RUN_TEST(test_tx_update_progress_retry_success_and_fail_closed);RUN_TEST(test_rx_round_robin_delivery_validation_and_sink_policy);RUN_TEST(test_cancel_clear_and_multiple_runtime_instances);UNITY_END();}
void loop(){}
