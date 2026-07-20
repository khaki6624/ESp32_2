#ifndef EVENT_TO_NOTIFICATION_BRIDGE_H
#define EVENT_TO_NOTIFICATION_BRIDGE_H

#include <EventToNotificationAdapter.h>

// Integration-only wiring for dispatch cycles that own an explicit current time.
// The bridge adds no queue, allocation, business logic, or time source.
class EventToNotificationBridge final
{
public:
    explicit EventToNotificationBridge(EventToNotificationAdapter& adapter);
    RuntimeIntegrationResult onEvent(const Event& event, NotificationTimestamp now);

private:
    EventToNotificationAdapter& adapter_;
};

#endif
