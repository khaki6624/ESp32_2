#include "NotificationMessage.h"

NotificationMessage::NotificationMessage():notificationId_(0U),channelId_(0U),
 priority_(NotificationPriority::COUNT),createdAt_(0U),ttlMs_(0U),target_(nullptr),
 targetLength_(0U),payload_(nullptr),payloadLength_(0U),valid_(false){}
NotificationMessage NotificationMessage::create(NotificationId id,
 NotificationChannelId channelId,NotificationPriority priority,
 NotificationTimestamp createdAt,NotificationDuration ttlMs,const char* target,
 size_t targetLength,const uint8_t* payload,size_t payloadLength)
{
 NotificationMessage result;
 if(id==INVALID_NOTIFICATION_ID||channelId==INVALID_NOTIFICATION_CHANNEL_ID||
  !isValidNotificationPriority(priority)||ttlMs==0U||target==nullptr||
  targetLength==0U||targetLength>NOTIFICATION_RUNTIME_MAX_TARGET_LENGTH||
  payload==nullptr||payloadLength==0U||
  payloadLength>NOTIFICATION_RUNTIME_MAX_PAYLOAD_LENGTH)return result;
 result.notificationId_=id;result.channelId_=channelId;result.priority_=priority;
 result.createdAt_=createdAt;result.ttlMs_=ttlMs;result.target_=target;
 result.targetLength_=targetLength;result.payload_=payload;
 result.payloadLength_=payloadLength;result.valid_=true;return result;
}
bool NotificationMessage::isValid()const
{return valid_&&notificationId_!=0U&&channelId_!=0U&&isValidNotificationPriority(priority_)&&ttlMs_!=0U&&target_!=nullptr&&targetLength_>0U&&targetLength_<=NOTIFICATION_RUNTIME_MAX_TARGET_LENGTH&&payload_!=nullptr&&payloadLength_>0U&&payloadLength_<=NOTIFICATION_RUNTIME_MAX_PAYLOAD_LENGTH;}
NotificationId NotificationMessage::notificationId()const{return notificationId_;}
NotificationChannelId NotificationMessage::channelId()const{return channelId_;}
NotificationPriority NotificationMessage::priority()const{return priority_;}
NotificationTimestamp NotificationMessage::createdAt()const{return createdAt_;}
NotificationDuration NotificationMessage::ttlMs()const{return ttlMs_;}
const char* NotificationMessage::target()const{return target_;}
size_t NotificationMessage::targetLength()const{return targetLength_;}
const uint8_t* NotificationMessage::payload()const{return payload_;}
size_t NotificationMessage::payloadLength()const{return payloadLength_;}
bool NotificationMessage::isExpired(NotificationTimestamp now)const
{return isValid()&&static_cast<NotificationDuration>(now-createdAt_)>=ttlMs_;}
