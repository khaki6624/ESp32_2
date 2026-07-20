#include "EventToNotificationBridge.h"

EventToNotificationBridge::EventToNotificationBridge(EventToNotificationAdapter& adapter) :
    adapter_(adapter) {}

RuntimeIntegrationResult EventToNotificationBridge::onEvent(
    const Event& event,
    NotificationTimestamp now)
{
    return adapter_.onEvent(event, now);
}
