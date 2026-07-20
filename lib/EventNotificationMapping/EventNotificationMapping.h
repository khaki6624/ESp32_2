#ifndef EVENT_NOTIFICATION_MAPPING_H
#define EVENT_NOTIFICATION_MAPPING_H
#include <NotificationRuntimeCommon.h>
#include <RuntimeIntegrationCommon.h>
class EventNotificationMapping
{public:EventNotificationMapping();static EventNotificationMapping create(EventType,EventSeverity,NotificationChannelId,NotificationPriority,NotificationDuration,const char*,size_t,bool);bool isValid()const;EventType eventType()const;EventSeverity minimumSeverity()const;NotificationChannelId channelId()const;NotificationPriority priority()const;NotificationDuration ttlMs()const;const char* target()const;size_t targetLength()const;bool enabled()const;
private:EventType eventType_;EventSeverity minimumSeverity_;NotificationChannelId channelId_;NotificationPriority priority_;NotificationDuration ttlMs_;const char* target_;size_t targetLength_;bool enabled_;bool valid_;};
#endif
