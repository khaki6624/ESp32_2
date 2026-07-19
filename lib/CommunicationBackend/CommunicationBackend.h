#ifndef COMMUNICATION_BACKEND_H
#define COMMUNICATION_BACKEND_H
#include <CommunicationMessage.h>
class CommunicationBackend
{
public:
 virtual ~CommunicationBackend()=default;
 virtual CommunicationBackendId backendId()const=0;
 virtual CommunicationType communicationType()const=0;
 virtual CommunicationBackendResult begin()=0;
 virtual bool isReady()const=0;
 virtual CommunicationBackendResult startSend(const CommunicationMessage& message,size_t& transferredLength)=0;
 virtual CommunicationBackendResult updateSend(size_t& transferredLength)=0;
 virtual CommunicationBackendResult cancelSend()=0;
 virtual CommunicationBackendResult pollReceive(CommunicationMessageId messageId,
  CommunicationWritePayload& receiveBuffer,CommunicationAddress& source,size_t& receivedLength)=0;
};
#endif
