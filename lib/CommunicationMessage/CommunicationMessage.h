#ifndef COMMUNICATION_MESSAGE_H
#define COMMUNICATION_MESSAGE_H
#include <CommunicationAddress.h>
#include <CommunicationPayload.h>
class CommunicationMessage
{
public:
    CommunicationMessage();
    static CommunicationMessage outbound(CommunicationMessageId messageId,
        CommunicationBackendId backendId, CommunicationType type,
        const CommunicationAddress& destination, const CommunicationReadPayload& payload);
    static CommunicationMessage inbound(CommunicationMessageId messageId,
        CommunicationBackendId backendId, CommunicationType type,
        const CommunicationAddress& source, const CommunicationWritePayload& payload,
        size_t receivedLength);
    bool isValid() const;
    CommunicationMessageId messageId() const; CommunicationBackendId backendId() const;
    CommunicationType communicationType() const; CommunicationDirection direction() const;
    const CommunicationAddress& address() const;
    const CommunicationReadPayload& outboundPayload() const;
    const CommunicationWritePayload& inboundPayload() const;
    size_t receivedLength() const;
private:
    CommunicationMessageId messageId_; CommunicationBackendId backendId_;
    CommunicationType type_; CommunicationDirection direction_;
    CommunicationAddress address_; CommunicationReadPayload outboundPayload_;
    CommunicationWritePayload inboundPayload_; size_t receivedLength_; bool valid_;
};
#endif
