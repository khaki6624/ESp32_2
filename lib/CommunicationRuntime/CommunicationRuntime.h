#ifndef COMMUNICATION_RUNTIME_H
#define COMMUNICATION_RUNTIME_H
#include <CommunicationRegistry.h>
#include <CommunicationTransaction.h>
#include <InboundMessageSink.h>

struct CommunicationRuntimeSlot
{
 CommunicationBackend* backend;
 CommunicationMessage activeMessage;
 CommunicationTransaction transaction;
 bool busy;
 CommunicationRuntimeSlot():backend(nullptr),activeMessage{},transaction{},busy(false){}
};

class CommunicationRuntime
{
public:
 CommunicationRuntime(CommunicationRegistry& registry,InboundMessageSink& sink,
  CommunicationWritePayload receiveBuffer);
 CommunicationResult begin(); bool isInitialized()const;
 CommunicationResult send(const CommunicationMessage& message);
 CommunicationResult update();
 CommunicationResult cancel(CommunicationBackendId backendId);
 bool isBackendBusy(CommunicationBackendId backendId)const;
 const CommunicationTransaction* transaction(CommunicationBackendId backendId)const;
 CommunicationResult clearCompleted(CommunicationBackendId backendId);
private:
 CommunicationRegistry& registry_; InboundMessageSink& sink_;
 CommunicationWritePayload receiveBuffer_;
 CommunicationRuntimeSlot slots_[COMMUNICATION_MAX_BACKENDS];
 size_t slotCount_; size_t nextReceiveBackendIndex_;
 CommunicationMessageId nextInboundMessageId_; bool initialized_;
 size_t findSlot(CommunicationBackendId id)const;
 void setTransaction(CommunicationRuntimeSlot& slot,const CommunicationMessage& message,
  CommunicationTransactionState state,CommunicationResult result,size_t transferred);
 CommunicationResult fail(CommunicationRuntimeSlot& slot,CommunicationResult result);
 bool validTransferred(const CommunicationMessage& message,size_t transferred)const;
 CommunicationResult updateTx(CommunicationRuntimeSlot& slot);
 CommunicationResult pollOneReceive();
 CommunicationMessageId consumeInboundMessageId();
};
#endif
