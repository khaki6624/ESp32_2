#ifndef NOTIFICATION_CHANNEL_DESCRIPTOR_H
#define NOTIFICATION_CHANNEL_DESCRIPTOR_H
#include <NotificationRuntimeCommon.h>
class NotificationChannelDescriptor
{
public:
 NotificationChannelDescriptor();
 static NotificationChannelDescriptor create(NotificationChannelId channelId,
  NotificationChannelType type,bool enabled);
 bool isValid()const; NotificationChannelId channelId()const;
 NotificationChannelType channelType()const; bool enabled()const;
private:
 NotificationChannelId channelId_; NotificationChannelType type_; bool enabled_; bool valid_;
};
#endif
