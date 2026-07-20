#ifndef NOTIFICATION_SINK_H
#define NOTIFICATION_SINK_H
#include <NotificationChannelDescriptor.h>
#include <NotificationMessage.h>
class NotificationSink
{
public:
 virtual ~NotificationSink()=default;
 virtual NotificationSinkResult sendNotification(const NotificationMessage& message,const NotificationChannelDescriptor& channel)=0;
 virtual NotificationSinkResult updateNotificationSend(NotificationChannelId channelId)=0;
 virtual NotificationSinkResult cancelNotificationSend(NotificationChannelId channelId)=0;
};
#endif
