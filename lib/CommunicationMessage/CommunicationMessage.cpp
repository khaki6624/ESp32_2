#include "CommunicationMessage.h"
CommunicationMessage::CommunicationMessage():messageId_(0U),backendId_(0U),
 type_(CommunicationType::COUNT),direction_(CommunicationDirection::COUNT),address_{},
 outboundPayload_{},inboundPayload_{},receivedLength_(0U),valid_(false){}
CommunicationMessage CommunicationMessage::outbound(CommunicationMessageId mid,
 CommunicationBackendId bid,CommunicationType type,const CommunicationAddress& address,
 const CommunicationReadPayload& payload)
{
 CommunicationMessage r;if(mid==0U||bid==0U||!isValidCommunicationType(type)||!address.isValid()||!payload.isValid())return r;
 r.messageId_=mid;r.backendId_=bid;r.type_=type;r.direction_=CommunicationDirection::OUTBOUND;
 r.address_=address;r.outboundPayload_=payload;r.valid_=true;return r;
}
CommunicationMessage CommunicationMessage::inbound(CommunicationMessageId mid,
 CommunicationBackendId bid,CommunicationType type,const CommunicationAddress& address,
 const CommunicationWritePayload& payload,size_t length)
{
 CommunicationMessage r;if(mid==0U||bid==0U||!isValidCommunicationType(type)||!address.isValid()||
  !payload.isValid()||length==0U||length>payload.capacity())return r;
 r.messageId_=mid;r.backendId_=bid;r.type_=type;r.direction_=CommunicationDirection::INBOUND;
 r.address_=address;r.inboundPayload_=payload;r.receivedLength_=length;r.valid_=true;return r;
}
bool CommunicationMessage::isValid()const
{
 if(!valid_||messageId_==0U||backendId_==0U||!isValidCommunicationType(type_)||!address_.isValid())return false;
 if(direction_==CommunicationDirection::OUTBOUND)return outboundPayload_.isValid()&&!inboundPayload_.isValid()&&receivedLength_==0U;
 if(direction_==CommunicationDirection::INBOUND)return !outboundPayload_.isValid()&&inboundPayload_.isValid()&&receivedLength_>0U&&receivedLength_<=inboundPayload_.capacity();
 return false;
}
CommunicationMessageId CommunicationMessage::messageId()const{return messageId_;} CommunicationBackendId CommunicationMessage::backendId()const{return backendId_;}
CommunicationType CommunicationMessage::communicationType()const{return type_;} CommunicationDirection CommunicationMessage::direction()const{return direction_;}
const CommunicationAddress& CommunicationMessage::address()const{return address_;} const CommunicationReadPayload& CommunicationMessage::outboundPayload()const{return outboundPayload_;}
const CommunicationWritePayload& CommunicationMessage::inboundPayload()const{return inboundPayload_;} size_t CommunicationMessage::receivedLength()const{return receivedLength_;}
