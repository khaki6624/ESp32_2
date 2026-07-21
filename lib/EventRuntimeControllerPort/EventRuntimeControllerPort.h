#ifndef EVENT_RUNTIME_CONTROLLER_PORT_H
#define EVENT_RUNTIME_CONTROLLER_PORT_H

#include <EventToNotificationDispatchBinding.h>
#include <MainControllerDependencies.h>

class EventRuntimeControllerPortTestAccess;

// The EVENT MainController port is the sole production owner of
// EventDispatcher execution. It delegates exactly once per controller cycle
// to EventToNotificationDispatchBinding::dispatchEvents(now).
// No other runtime or component may drain the EventQueue directly.
class EventRuntimeControllerPort final : public MainControllerComponentPort
{
public:
    explicit EventRuntimeControllerPort(EventToNotificationDispatchBinding& binding);

    MainControllerComponent component() const override;
    bool critical() const override;
    bool hasBegin() const override;
    bool hasUpdate() const override;
    MainControllerResult begin(MainControllerTimestamp now) override;
    MainControllerResult update(MainControllerTimestamp now) override;

private:
    friend class EventRuntimeControllerPortTestAccess;
    EventToNotificationDispatchBinding& binding_;

    static MainControllerResult mapResult(RuntimeIntegrationResult result);
};

#endif
