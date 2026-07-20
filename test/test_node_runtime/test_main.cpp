#include <Arduino.h>
#include <unity.h>
#include <string.h>
#include <NodeRuntime.h>

class FakeNodeMessageSink final:public NodeMessageSink
{
public:
 NodeMessageSinkResult result=NodeMessageSinkResult::ACCEPTED; size_t calls=0; NodeMessageId messageId=0; NodeId sourceId=0; NodeAddress address{}; size_t length=0; uint8_t payload[NODE_RUNTIME_MAX_PAYLOAD_LENGTH]={};
 NodeMessageSinkResult onNodeMessage(const NodeInboundMessage& m)override{++calls;messageId=m.messageId();sourceId=m.sourceNodeId();address=m.sourceAddress();length=m.payloadLength();memcpy(payload,m.payload(),length);return result;}
};
class FakeNodeOutboundSink final:public NodeOutboundSink
{
public:
 NodeOutboundSinkResult sendResult=NodeOutboundSinkResult::SUCCESS,updateResult=NodeOutboundSinkResult::SUCCESS,cancelResult=NodeOutboundSinkResult::SUCCESS;size_t sendCalls=0,updateCalls=0,cancelCalls=0;NodeId nodeId=0;NodeMessageId messageId=0;size_t length=0;uint8_t payload[NODE_RUNTIME_MAX_PAYLOAD_LENGTH]={};
 NodeOutboundSinkResult sendToNode(NodeMessageId mid,const NodeDescriptor& d,const uint8_t* p,size_t n)override{++sendCalls;nodeId=d.nodeId();messageId=mid;length=n;memcpy(payload,p,n);return sendResult;}
 NodeOutboundSinkResult updateNodeSend(NodeId id)override{++updateCalls;nodeId=id;return updateResult;}
 NodeOutboundSinkResult cancelNodeSend(NodeId id)override{++cancelCalls;nodeId=id;return cancelResult;}
};
static NodeAddress address(const char* text){NodeAddress a;TEST_ASSERT_TRUE(a.set(text));return a;}
static NodeDescriptor descriptor(NodeId id,const char* a,bool enabled=true,NodeDuration timeout=100U){return NodeDescriptor::create(id,NodeType::CUSTOM,address(a),timeout,enabled);}
static void online(NodeRuntime& runtime,NodeId id,const char* a,NodeTimestamp now){const uint8_t p[]={0x41,0x00,0x42};TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::SUCCESS,(uint8_t)runtime.handleInbound(NodeInboundMessage::create(10U+id,id,address(a),p,sizeof(p)),now));}

void test_address_descriptor_and_registry()
{
 NodeAddress a;TEST_ASSERT_FALSE(a.isValid());TEST_ASSERT_FALSE(a.set(nullptr));TEST_ASSERT_FALSE(a.set(""));TEST_ASSERT_TRUE(a.set("node/Main-1:+@"));TEST_ASSERT_FALSE(a.set("bad address"));TEST_ASSERT_EQUAL_STRING("node/Main-1:+@",a.c_str());NodeAddress lower=address("node/main-1:+@");TEST_ASSERT_FALSE(a.equals(lower));
 char longValue[NODE_RUNTIME_MAX_ADDRESS_LENGTH+1];memset(longValue,'a',sizeof(longValue));longValue[sizeof(longValue)-1]='\0';TEST_ASSERT_FALSE(a.set(longValue));TEST_ASSERT_EQUAL_STRING("node/Main-1:+@",a.c_str());
 TEST_ASSERT_FALSE(NodeDescriptor::create(0,NodeType::CUSTOM,a,1,true).isValid());TEST_ASSERT_FALSE(NodeDescriptor::create(1,NodeType::COUNT,a,1,true).isValid());TEST_ASSERT_FALSE(NodeDescriptor::create(1,NodeType::CUSTOM,a,0,true).isValid());
 NodeRuntimeRegistry r;TEST_ASSERT_EQUAL_UINT32(0,r.size());TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::SUCCESS,(uint8_t)r.registerNode(descriptor(1,"node/1")));TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::DUPLICATE_NODE_ID,(uint8_t)r.registerNode(descriptor(1,"node/2")));TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::DUPLICATE_NODE_ADDRESS,(uint8_t)r.registerNode(descriptor(2,"node/1")));TEST_ASSERT_NOT_NULL(r.findById(1));TEST_ASSERT_NOT_NULL(r.findByAddress(address("node/1")));TEST_ASSERT_NULL(r.findById(99));TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::SUCCESS,(uint8_t)r.lock());TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::REGISTRY_LOCKED,(uint8_t)r.registerNode(descriptor(2,"node/2")));
}
void test_inbound_timeout_recovery_and_wraparound()
{
 NodeRuntimeRegistry r;r.registerNode(descriptor(1,"node/main",true,100));r.registerNode(descriptor(2,"node/off",false,10));FakeNodeMessageSink in;FakeNodeOutboundSink out;NodeRuntime runtime(r,in,out);TEST_ASSERT_FALSE(runtime.isInitialized());TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::NOT_INITIALIZED,(uint8_t)runtime.update(0));TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::SUCCESS,(uint8_t)runtime.begin(1000));TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeConnectionState::UNKNOWN,(uint8_t)runtime.state(1)->connectionState());TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeConnectionState::DISABLED,(uint8_t)runtime.state(2)->connectionState());TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::NO_CHANGE,(uint8_t)runtime.update(5000));
 const uint8_t binary[]={0x41,0,0x42};NodeInboundMessage m=NodeInboundMessage::create(77,1,address("node/main"),binary,sizeof(binary));TEST_ASSERT_TRUE(m.isValid());TEST_ASSERT_EQUAL_PTR(binary,m.payload());TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::SUCCESS,(uint8_t)runtime.handleInbound(m,0xFFFFFFF0U));TEST_ASSERT_TRUE(runtime.isOnline(1));TEST_ASSERT_EQUAL_UINT32(1,in.calls);TEST_ASSERT_EQUAL_UINT8_ARRAY(binary,in.payload,sizeof(binary));TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::NO_CHANGE,(uint8_t)runtime.update(0x00000040U));TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::TIMEOUT,(uint8_t)runtime.update(0x00000054U));TEST_ASSERT_FALSE(runtime.isOnline(1));NodeTimestamp changed=runtime.state(1)->lastStateChangeAt();TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::NO_CHANGE,(uint8_t)runtime.update(0x60));TEST_ASSERT_EQUAL_UINT32(changed,runtime.state(1)->lastStateChangeAt());online(runtime,1,"node/main",200);TEST_ASSERT_TRUE(runtime.isOnline(1));
 in.result=NodeMessageSinkResult::REJECTED;TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::SINK_REJECTED,(uint8_t)runtime.handleInbound(NodeInboundMessage::create(88,1,address("node/main"),binary,3),210));TEST_ASSERT_TRUE(runtime.isOnline(1));size_t calls=in.calls;TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::INVALID_MESSAGE,(uint8_t)runtime.handleInbound(NodeInboundMessage::create(89,1,address("wrong"),binary,3),220));TEST_ASSERT_EQUAL_UINT32(calls,in.calls);
}
void test_outbound_nonblocking_busy_cancel_and_priority()
{
 NodeRuntimeRegistry r;r.registerNode(descriptor(1,"node/1",true,50));r.registerNode(descriptor(2,"node/2",true,1000));FakeNodeMessageSink in;FakeNodeOutboundSink out;NodeRuntime runtime(r,in,out);runtime.begin(0);online(runtime,1,"node/1",10);online(runtime,2,"node/2",10);const uint8_t p[]={0x10,0,0xff};
 out.sendResult=NodeOutboundSinkResult::IN_PROGRESS;TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::ACCEPTED,(uint8_t)runtime.send(200,1,p,3));TEST_ASSERT_TRUE(runtime.isBusy(1));TEST_ASSERT_EQUAL_UINT32(200,runtime.state(1)->activeOutboundMessageId());TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::NODE_BUSY,(uint8_t)runtime.send(201,1,p,3));TEST_ASSERT_EQUAL_UINT32(1,out.sendCalls);TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::ACCEPTED,(uint8_t)runtime.send(200,2,p,3));TEST_ASSERT_TRUE(runtime.isBusy(2));
 out.updateResult=NodeOutboundSinkResult::IN_PROGRESS;TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::TIMEOUT,(uint8_t)runtime.update(60));TEST_ASSERT_EQUAL_UINT32(2,out.updateCalls);out.updateResult=NodeOutboundSinkResult::SUCCESS;TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::SUCCESS,(uint8_t)runtime.update(61));TEST_ASSERT_FALSE(runtime.isBusy(1));TEST_ASSERT_FALSE(runtime.isBusy(2));TEST_ASSERT_EQUAL_UINT32(0,runtime.state(1)->activeOutboundMessageId());
 online(runtime,1,"node/1",70);out.sendResult=NodeOutboundSinkResult::ACCEPTED;runtime.send(300,1,p,3);out.cancelResult=NodeOutboundSinkResult::IN_PROGRESS;TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::IN_PROGRESS,(uint8_t)runtime.cancelSend(1));TEST_ASSERT_TRUE(runtime.isBusy(1));out.cancelResult=NodeOutboundSinkResult::REJECTED;TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::SINK_REJECTED,(uint8_t)runtime.cancelSend(1));TEST_ASSERT_FALSE(runtime.isBusy(1));TEST_ASSERT_EQUAL_UINT32(0,runtime.state(1)->activeOutboundMessageId());
 TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::TIMEOUT,(uint8_t)runtime.update(120));TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::NODE_OFFLINE,(uint8_t)runtime.send(400,1,p,3));TEST_ASSERT_EQUAL_UINT8((uint8_t)NodeRuntimeResult::INVALID_ARGUMENT,(uint8_t)runtime.cancelSend(1));
}
void test_multiple_instances_are_independent(){NodeRuntimeRegistry a,b;a.registerNode(descriptor(1,"a"));b.registerNode(descriptor(2,"b"));FakeNodeMessageSink ia,ib;FakeNodeOutboundSink oa,ob;NodeRuntime ra(a,ia,oa),rb(b,ib,ob);ra.begin(1);rb.begin(2);online(ra,1,"a",3);TEST_ASSERT_TRUE(ra.isOnline(1));TEST_ASSERT_FALSE(rb.isOnline(2));TEST_ASSERT_NULL(rb.state(1));}

void setup(){delay(2000);UNITY_BEGIN();RUN_TEST(test_address_descriptor_and_registry);RUN_TEST(test_inbound_timeout_recovery_and_wraparound);RUN_TEST(test_outbound_nonblocking_busy_cancel_and_priority);RUN_TEST(test_multiple_instances_are_independent);UNITY_END();}
void loop(){}
