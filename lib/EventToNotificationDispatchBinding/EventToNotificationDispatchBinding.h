#ifndef EVENT_TO_NOTIFICATION_DISPATCH_BINDING_H
#define EVENT_TO_NOTIFICATION_DISPATCH_BINDING_H

#include <EventDispatcher.h>
#include <EventToNotificationBridge.h>

class EventToNotificationDispatchBindingTestAccess;

// Composition-root-owned binding for the synchronous EventDispatcher contract.
// Dispatch time is valid only between beginDispatch() and endDispatch().
class EventToNotificationDispatchBinding final : public EventHandler
{
public:
    EventToNotificationDispatchBinding(
        EventQueue& queue,
        EventDispatcher& dispatcher,
        EventToNotificationBridge& bridge);

    RuntimeIntegrationResult begin();
    RuntimeIntegrationResult dispatchEvents(NotificationTimestamp now);
    RuntimeIntegrationResult beginDispatch(NotificationTimestamp now);
    RuntimeIntegrationResult endDispatch();
    EventHandleResult handle(const Event& event) override;
    bool isInitialized() const;
    bool isDispatching() const;

private:
    friend class EventToNotificationDispatchBindingTestAccess;
    EventQueue& queue_;
    EventDispatcher& dispatcher_;
    EventToNotificationBridge& bridge_;
    NotificationTimestamp currentDispatchNow_;
    bool initialized_;
    bool dispatchActive_;

    static EventHandleResult mapHandlerResult(RuntimeIntegrationResult result);
    static RuntimeIntegrationResult mapDispatchResult(EventDispatchResult result);
    static uint8_t dispatchPriority(RuntimeIntegrationResult result);
    static RuntimeIntegrationResult combineDispatchResults(
        RuntimeIntegrationResult current,
        RuntimeIntegrationResult candidate);
};

#endif
