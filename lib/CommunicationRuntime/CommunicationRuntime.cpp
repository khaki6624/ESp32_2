#include "CommunicationRuntime.h"
namespace
{
constexpr size_t INVALID_SLOT=COMMUNICATION_MAX_BACKENDS;
uint8_t updatePriority(CommunicationResult r)
{
 // Priority: INTERNAL_ERROR > BACKEND_FAILED > SINK_REJECTED > CANCELLED >
 // SUCCESS > IN_PROGRESS > BACKEND_RETRY_LATER > NO_MESSAGE.
 if(r==CommunicationResult::INTERNAL_ERROR)return 8U;
 if(r==CommunicationResult::BACKEND_FAILED)return 7U;
 if(r==CommunicationResult::SINK_REJECTED)return 6U;
 if(r==CommunicationResult::CANCELLED)return 5U;
 if(r==CommunicationResult::SUCCESS)return 4U;
 if(r==CommunicationResult::IN_PROGRESS)return 3U;
 if(r==CommunicationResult::BACKEND_RETRY_LATER)return 2U;
 return 1U;
}
CommunicationResult combine(CommunicationResult current,CommunicationResult candidate)
{return updatePriority(candidate)>updatePriority(current)?candidate:current;}
uint8_t beginPriority(CommunicationResult r)
{
 if(r==CommunicationResult::INTERNAL_ERROR)return 5U;
 if(r==CommunicationResult::BACKEND_FAILED)return 4U;
 if(r==CommunicationResult::BACKEND_RETRY_LATER)return 3U;
 if(r==CommunicationResult::BACKEND_NOT_READY)return 2U;
 return 1U;
}
}
CommunicationRuntime::CommunicationRuntime(CommunicationRegistry& registry,InboundMessageSink& sink,
 CommunicationWritePayload receiveBuffer):registry_(registry),sink_(sink),receiveBuffer_(receiveBuffer),
 slots_{},slotCount_(0U),nextReceiveBackendIndex_(0U),nextInboundMessageId_(1U),initialized_(false){}
size_t CommunicationRuntime::findSlot(CommunicationBackendId id)const
{for(size_t i=0U;i<slotCount_;++i)if(slots_[i].backend!=nullptr&&slots_[i].backend->backendId()==id)return i;return INVALID_SLOT;}
CommunicationResult CommunicationRuntime::begin()
{
 for(size_t i=0U;i<slotCount_;++i)if(slots_[i].busy)return CommunicationResult::BUSY;
 initialized_=false;
 if(!receiveBuffer_.isValid())return CommunicationResult::INVALID_PAYLOAD;
 registry_.lock(); slotCount_=registry_.size();
 if(slotCount_>COMMUNICATION_MAX_BACKENDS)return CommunicationResult::INTERNAL_ERROR;
 for(size_t i=0U;i<COMMUNICATION_MAX_BACKENDS;++i)slots_[i]=CommunicationRuntimeSlot{};
 nextReceiveBackendIndex_=0U;nextInboundMessageId_=1U;
 CommunicationResult overall=CommunicationResult::SUCCESS;
 for(size_t i=0U;i<slotCount_;++i)
 {
  CommunicationBackend* b=registry_.at(i);slots_[i].backend=b;
  if(b==nullptr||b->backendId()==0U||!isValidCommunicationType(b->communicationType()))
  {overall=CommunicationResult::INTERNAL_ERROR;continue;}
  CommunicationBackendResult r=b->begin();CommunicationResult mapped=CommunicationResult::SUCCESS;
  if(!isValidCommunicationBackendResult(r))mapped=CommunicationResult::INTERNAL_ERROR;
  else if(r==CommunicationBackendResult::FAILED||r==CommunicationBackendResult::CANCELLED||r==CommunicationBackendResult::NO_MESSAGE)mapped=CommunicationResult::BACKEND_FAILED;
  else if(r==CommunicationBackendResult::RETRY_LATER)mapped=CommunicationResult::BACKEND_RETRY_LATER;
  else if(r==CommunicationBackendResult::ACCEPTED||r==CommunicationBackendResult::IN_PROGRESS)mapped=CommunicationResult::BACKEND_NOT_READY;
  if(beginPriority(mapped)>beginPriority(overall))overall=mapped;
 }
 initialized_=overall==CommunicationResult::SUCCESS;return overall;
}
bool CommunicationRuntime::isInitialized()const{return initialized_;}
void CommunicationRuntime::setTransaction(CommunicationRuntimeSlot& s,const CommunicationMessage& m,
 CommunicationTransactionState state,CommunicationResult result,size_t transferred)
{s.transaction.messageId_=m.messageId();s.transaction.backendId_=m.backendId();s.transaction.type_=m.communicationType();s.transaction.state_=state;s.transaction.result_=result;s.transaction.transferredLength_=transferred;s.transaction.valid_=true;}
CommunicationResult CommunicationRuntime::fail(CommunicationRuntimeSlot& s,CommunicationResult r)
{s.transaction.state_=CommunicationTransactionState::FAILED;s.transaction.result_=r;s.busy=false;return r;}
bool CommunicationRuntime::validTransferred(const CommunicationMessage& m,size_t n)const
{return m.isValid()&&m.direction()==CommunicationDirection::OUTBOUND&&n<=m.outboundPayload().length();}
CommunicationResult CommunicationRuntime::send(const CommunicationMessage& m)
{
 if(!initialized_)return CommunicationResult::NOT_INITIALIZED;
 if(!m.isValid()||m.direction()!=CommunicationDirection::OUTBOUND)return CommunicationResult::INVALID_MESSAGE;
 size_t index=findSlot(m.backendId());if(index==INVALID_SLOT)return CommunicationResult::BACKEND_NOT_FOUND;
 CommunicationRuntimeSlot& s=slots_[index];if(s.backend->communicationType()!=m.communicationType())return CommunicationResult::INVALID_MESSAGE;
 if(!s.backend->isReady())return CommunicationResult::BACKEND_NOT_READY;if(s.busy)return CommunicationResult::BUSY;
 size_t transferred=0U;CommunicationBackendResult br=s.backend->startSend(m,transferred);
 bool accepted=br==CommunicationBackendResult::SUCCESS||br==CommunicationBackendResult::ACCEPTED||br==CommunicationBackendResult::IN_PROGRESS;
 if(!validTransferred(m,transferred)||(!accepted&&transferred!=0U)||!isValidCommunicationBackendResult(br))
 {setTransaction(s,m,CommunicationTransactionState::FAILED,CommunicationResult::INTERNAL_ERROR,transferred);return CommunicationResult::INTERNAL_ERROR;}
 if(br==CommunicationBackendResult::SUCCESS){s.activeMessage=m;setTransaction(s,m,CommunicationTransactionState::SUCCEEDED,CommunicationResult::SUCCESS,transferred);return CommunicationResult::SUCCESS;}
 if(br==CommunicationBackendResult::ACCEPTED||br==CommunicationBackendResult::IN_PROGRESS)
 {s.activeMessage=m;s.busy=true;CommunicationTransactionState st=br==CommunicationBackendResult::ACCEPTED?CommunicationTransactionState::PENDING:CommunicationTransactionState::RUNNING;
  setTransaction(s,m,st,br==CommunicationBackendResult::ACCEPTED?CommunicationResult::ACCEPTED:CommunicationResult::IN_PROGRESS,transferred);return CommunicationResult::ACCEPTED;}
 CommunicationResult r=br==CommunicationBackendResult::RETRY_LATER?CommunicationResult::BACKEND_RETRY_LATER:
  br==CommunicationBackendResult::FAILED?CommunicationResult::BACKEND_FAILED:
  br==CommunicationBackendResult::CANCELLED?CommunicationResult::CANCELLED:CommunicationResult::INTERNAL_ERROR;
 CommunicationTransactionState st=r==CommunicationResult::CANCELLED?CommunicationTransactionState::CANCELLED:CommunicationTransactionState::FAILED;
 setTransaction(s,m,st,r,0U);return r;
}
CommunicationResult CommunicationRuntime::updateTx(CommunicationRuntimeSlot& s)
{
 size_t n=s.transaction.transferredLength_;CommunicationBackendResult br=s.backend->updateSend(n);
 if(n<s.transaction.transferredLength_||!validTransferred(s.activeMessage,n)||!isValidCommunicationBackendResult(br))return fail(s,CommunicationResult::INTERNAL_ERROR);
 s.transaction.transferredLength_=n;
 if(br==CommunicationBackendResult::IN_PROGRESS){s.transaction.state_=CommunicationTransactionState::RUNNING;s.transaction.result_=CommunicationResult::IN_PROGRESS;return CommunicationResult::IN_PROGRESS;}
 if(br==CommunicationBackendResult::RETRY_LATER){s.transaction.state_=CommunicationTransactionState::RUNNING;s.transaction.result_=CommunicationResult::BACKEND_RETRY_LATER;return CommunicationResult::BACKEND_RETRY_LATER;}
 if(br==CommunicationBackendResult::SUCCESS){s.transaction.state_=CommunicationTransactionState::SUCCEEDED;s.transaction.result_=CommunicationResult::SUCCESS;s.busy=false;return CommunicationResult::SUCCESS;}
 if(br==CommunicationBackendResult::FAILED)return fail(s,CommunicationResult::BACKEND_FAILED);
 if(br==CommunicationBackendResult::CANCELLED){s.transaction.state_=CommunicationTransactionState::CANCELLED;s.transaction.result_=CommunicationResult::CANCELLED;s.busy=false;return CommunicationResult::CANCELLED;}
 return fail(s,CommunicationResult::INTERNAL_ERROR);
}
CommunicationMessageId CommunicationRuntime::consumeInboundMessageId()
{CommunicationMessageId id=nextInboundMessageId_++;if(nextInboundMessageId_==0U)nextInboundMessageId_=1U;return id;}
CommunicationResult CommunicationRuntime::pollOneReceive()
{
 if(slotCount_==0U)return CommunicationResult::NO_MESSAGE;
 for(size_t offset=0U;offset<slotCount_;++offset)
 {
  size_t i=(nextReceiveBackendIndex_+offset)%slotCount_;CommunicationBackend* b=slots_[i].backend;
  if(b==nullptr||!b->isReady())continue;
  nextReceiveBackendIndex_=(i+1U)%slotCount_;
  CommunicationWritePayload buffer=receiveBuffer_;CommunicationAddress source;size_t length=0U;
  CommunicationBackendResult br=b->pollReceive(nextInboundMessageId_,buffer,source,length);
  if(buffer.data()!=receiveBuffer_.data()||buffer.capacity()!=receiveBuffer_.capacity())return CommunicationResult::INTERNAL_ERROR;
  if(!isValidCommunicationBackendResult(br))return CommunicationResult::INTERNAL_ERROR;
  if(br!=CommunicationBackendResult::SUCCESS)
  {
   if(length!=0U)return CommunicationResult::INTERNAL_ERROR;
   if(br==CommunicationBackendResult::NO_MESSAGE)return CommunicationResult::NO_MESSAGE;
   if(br==CommunicationBackendResult::RETRY_LATER)return CommunicationResult::BACKEND_RETRY_LATER;
   if(br==CommunicationBackendResult::FAILED)return CommunicationResult::BACKEND_FAILED;
   return CommunicationResult::INTERNAL_ERROR;
  }
  if(length==0U||length>receiveBuffer_.capacity()||!source.isValid()||b->backendId()==0U||!isValidCommunicationType(b->communicationType()))return CommunicationResult::INTERNAL_ERROR;
  CommunicationMessage message=CommunicationMessage::inbound(consumeInboundMessageId(),b->backendId(),b->communicationType(),source,receiveBuffer_,length);
  if(!message.isValid())return CommunicationResult::INTERNAL_ERROR;
  InboundMessageSinkResult sr=sink_.onMessage(message);
  if(!isValidInboundMessageSinkResult(sr))return CommunicationResult::INTERNAL_ERROR;
  return sr==InboundMessageSinkResult::ACCEPTED?CommunicationResult::SUCCESS:CommunicationResult::SINK_REJECTED;
 }
 return CommunicationResult::NO_MESSAGE;
}
CommunicationResult CommunicationRuntime::update()
{
 if(!initialized_)return CommunicationResult::NOT_INITIALIZED;CommunicationResult overall=CommunicationResult::NO_MESSAGE;
 for(size_t i=0U;i<slotCount_;++i)if(slots_[i].busy)overall=combine(overall,updateTx(slots_[i]));
 return combine(overall,pollOneReceive());
}
CommunicationResult CommunicationRuntime::cancel(CommunicationBackendId id)
{
 if(!initialized_)return CommunicationResult::NOT_INITIALIZED;size_t i=findSlot(id);if(i==INVALID_SLOT)return CommunicationResult::BACKEND_NOT_FOUND;
 CommunicationRuntimeSlot& s=slots_[i];if(!s.busy)return CommunicationResult::INVALID_ARGUMENT;CommunicationBackendResult br=s.backend->cancelSend();
 if(!isValidCommunicationBackendResult(br))return fail(s,CommunicationResult::INTERNAL_ERROR);
 if(br==CommunicationBackendResult::SUCCESS||br==CommunicationBackendResult::CANCELLED){s.transaction.state_=CommunicationTransactionState::CANCELLED;s.transaction.result_=CommunicationResult::CANCELLED;s.busy=false;return CommunicationResult::CANCELLED;}
 if(br==CommunicationBackendResult::IN_PROGRESS||br==CommunicationBackendResult::RETRY_LATER){s.transaction.state_=CommunicationTransactionState::RUNNING;s.transaction.result_=CommunicationResult::BACKEND_RETRY_LATER;return CommunicationResult::BACKEND_RETRY_LATER;}
 if(br==CommunicationBackendResult::FAILED)return fail(s,CommunicationResult::BACKEND_FAILED);return fail(s,CommunicationResult::INTERNAL_ERROR);
}
bool CommunicationRuntime::isBackendBusy(CommunicationBackendId id)const{size_t i=findSlot(id);return i!=INVALID_SLOT&&slots_[i].busy;}
const CommunicationTransaction* CommunicationRuntime::transaction(CommunicationBackendId id)const{size_t i=findSlot(id);return i==INVALID_SLOT?nullptr:&slots_[i].transaction;}
CommunicationResult CommunicationRuntime::clearCompleted(CommunicationBackendId id)
{if(!initialized_)return CommunicationResult::NOT_INITIALIZED;size_t i=findSlot(id);if(i==INVALID_SLOT)return CommunicationResult::BACKEND_NOT_FOUND;if(slots_[i].busy)return CommunicationResult::BUSY;slots_[i].transaction=CommunicationTransaction{};return CommunicationResult::SUCCESS;}
