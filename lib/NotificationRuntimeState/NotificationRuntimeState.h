#ifndef NOTIFICATION_RUNTIME_STATE_H
#define NOTIFICATION_RUNTIME_STATE_H
#include <NotificationRuntimeCommon.h>
class NotificationRuntime;
class NotificationRuntimeState
{
public:
 NotificationRuntimeState();
 bool isValid()const; NotificationChannelId channelId()const;
 NotificationRuntimeResult lastResult()const; NotificationId activeNotificationId()const;
 bool busy()const;
private:
 friend class NotificationRuntime;
 NotificationChannelId channelId_; NotificationRuntimeResult lastResult_;
 NotificationId activeNotificationId_; bool busy_; bool valid_;
};
#endif
