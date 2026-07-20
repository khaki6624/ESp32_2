#include "NotificationChannelDescriptor.h"
NotificationChannelDescriptor::NotificationChannelDescriptor():channelId_(0U),type_(NotificationChannelType::COUNT),enabled_(false),valid_(false){}
NotificationChannelDescriptor NotificationChannelDescriptor::create(NotificationChannelId id,NotificationChannelType type,bool enabled)
{NotificationChannelDescriptor result;if(id==0U||!isValidNotificationChannelType(type))return result;result.channelId_=id;result.type_=type;result.enabled_=enabled;result.valid_=true;return result;}
bool NotificationChannelDescriptor::isValid()const{return valid_&&channelId_!=0U&&isValidNotificationChannelType(type_);}
NotificationChannelId NotificationChannelDescriptor::channelId()const{return channelId_;}
NotificationChannelType NotificationChannelDescriptor::channelType()const{return type_;}
bool NotificationChannelDescriptor::enabled()const{return enabled_;}
