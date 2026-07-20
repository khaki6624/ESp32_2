#ifndef EVENT_TO_NOTIFICATION_ADAPTER_H
#define EVENT_TO_NOTIFICATION_ADAPTER_H
#include <EventHandler.h>
#include <EventNotificationFormatter.h>
#include <EventNotificationRegistry.h>
#include <NotificationRuntime.h>
class EventToNotificationAdapterTestAccess;
class EventToNotificationAdapter final:public EventHandler
{public:EventToNotificationAdapter(EventNotificationRegistry&,EventNotificationFormatter&,NotificationRuntime&);RuntimeIntegrationResult begin();bool isInitialized()const;RuntimeIntegrationResult onEvent(const Event&,NotificationTimestamp);EventHandleResult handle(const Event&)override;uint32_t forwardedCount()const;uint32_t ignoredCount()const;uint32_t failedCount()const;NotificationId nextNotificationId()const;RuntimeIntegrationResult lastResult()const;
private:friend class EventToNotificationAdapterTestAccess;EventNotificationRegistry& registry_;EventNotificationFormatter& formatter_;NotificationRuntime& runtime_;uint8_t payload_[NOTIFICATION_RUNTIME_MAX_PAYLOAD_LENGTH];NotificationId nextId_;uint32_t forwarded_;uint32_t ignored_;uint32_t failed_;RuntimeIntegrationResult last_;bool initialized_;NotificationId consumeId();RuntimeIntegrationResult finish(RuntimeIntegrationResult);static RuntimeIntegrationResult mapRuntimeResult(NotificationRuntimeResult);};
#endif
