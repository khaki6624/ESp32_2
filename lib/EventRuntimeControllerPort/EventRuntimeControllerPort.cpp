#include "EventRuntimeControllerPort.h"

EventRuntimeControllerPort::EventRuntimeControllerPort(
    EventToNotificationDispatchBinding& binding) : binding_(binding) {}

MainControllerComponent EventRuntimeControllerPort::component() const
{
    return MainControllerComponent::EVENT;
}

bool EventRuntimeControllerPort::critical() const { return true; }
bool EventRuntimeControllerPort::hasBegin() const { return true; }
bool EventRuntimeControllerPort::hasUpdate() const { return true; }

MainControllerResult EventRuntimeControllerPort::mapResult(RuntimeIntegrationResult result)
{
    switch (result)
    {
        case RuntimeIntegrationResult::SUCCESS:
            return MainControllerResult::SUCCESS;
        case RuntimeIntegrationResult::NO_CHANGE:
        case RuntimeIntegrationResult::IGNORED:
        case RuntimeIntegrationResult::FILTERED:
        case RuntimeIntegrationResult::MAPPING_DISABLED:
            return MainControllerResult::NO_CHANGE;
        case RuntimeIntegrationResult::ACCEPTED:
        case RuntimeIntegrationResult::IN_PROGRESS:
            return MainControllerResult::IN_PROGRESS;
        case RuntimeIntegrationResult::RETRY_LATER:
            return MainControllerResult::RETRY_LATER;
        case RuntimeIntegrationResult::REJECTED:
        case RuntimeIntegrationResult::FAILED:
        case RuntimeIntegrationResult::NOT_INITIALIZED:
        case RuntimeIntegrationResult::INVALID_ARGUMENT:
        case RuntimeIntegrationResult::INVALID_INPUT:
        case RuntimeIntegrationResult::MAPPING_NOT_FOUND:
        case RuntimeIntegrationResult::REGISTRY_FULL:
        case RuntimeIntegrationResult::REGISTRY_LOCKED:
        case RuntimeIntegrationResult::DUPLICATE_MAPPING:
            return MainControllerResult::COMPONENT_FAILED;
        case RuntimeIntegrationResult::INTERNAL_ERROR:
        case RuntimeIntegrationResult::COUNT:
        default:
            return MainControllerResult::INTERNAL_ERROR;
    }
}

MainControllerResult EventRuntimeControllerPort::begin(MainControllerTimestamp now)
{
    (void)now;
    return mapResult(binding_.begin());
}

MainControllerResult EventRuntimeControllerPort::update(MainControllerTimestamp now)
{
    return mapResult(binding_.dispatchEvents(now));
}
