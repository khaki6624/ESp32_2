#include "RelayOutputAdapter.h"

RelayOutputAdapter::RelayOutputAdapter(Relay& relay) : relay_(relay) {}

DriverType RelayOutputAdapter::getDriverType() const
{
    return DriverType::RELAY;
}

bool RelayOutputAdapter::supportsAction(DeviceAction action) const
{
    switch (action)
    {
        case DeviceAction::ON:
        case DeviceAction::OFF:
        case DeviceAction::TOGGLE:
        case DeviceAction::PULSE:
            return true;
        default:
            return false;
    }
}

DeviceValueType RelayOutputAdapter::getValueType() const
{
    return DeviceValueType::BOOLEAN;
}

DriverPortHealth RelayOutputAdapter::getHealth() const
{
    // Relay فعلی قرارداد Health یا Ready مستقل ندارد.
    return DriverPortHealth::READY;
}

DriverExecutionResult RelayOutputAdapter::execute(
    const DriverActionRequest& request,
    DriverExecutionResponse& response
)
{
    if (!request.isValid() || request.action == DeviceAction::NONE)
        return DriverExecutionResult::INVALID_ACTION;
    if (request.action == DeviceAction::READ)
        return DriverExecutionResult::READ_NOT_SUPPORTED;
    if (!supportsAction(request.action))
        return DriverExecutionResult::ACTION_NOT_SUPPORTED;
    if (request.hasValue)
        return DriverExecutionResult::INVALID_VALUE;

    if (request.action == DeviceAction::PULSE)
    {
        if (!request.hasDuration || request.durationMs == 0U)
            return DriverExecutionResult::INVALID_DURATION;
    }
    else if (request.hasDuration)
    {
        return DriverExecutionResult::INVALID_DURATION;
    }

    switch (request.action)
    {
        case DeviceAction::ON:
            relay_.on();
            break;
        case DeviceAction::OFF:
            relay_.off();
            break;
        case DeviceAction::TOGGLE:
            relay_.toggle();
            break;
        case DeviceAction::PULSE:
            // فقط Pulse آغاز می‌شود؛ update و انتظار پایان بر عهده مالک Driver است.
            relay_.pulse(request.durationMs);
            break;
        default:
            return DriverExecutionResult::ACTION_NOT_SUPPORTED;
    }

    DriverExecutionResponse temporary;
    temporary.result = DriverExecutionResult::SUCCESS;
    temporary.actualValue = DeviceValue::makeBoolean(
        relay_.isOn(),
        request.requestedTimestampMs
    );
    temporary.hasActualValue = true;
    temporary.portHealth = DriverPortHealth::READY;
    temporary.completedTimestampMs = request.requestedTimestampMs;
    if (!temporary.isValid())
        return DriverExecutionResult::OUTPUT_VALUE_INVALID;

    response = temporary;
    return DriverExecutionResult::SUCCESS;
}

DriverExecutionResult RelayOutputAdapter::readActualValue(DeviceValue& output) const
{
    DeviceValue temporary = DeviceValue::makeBoolean(relay_.isOn(), 0U);
    if (!isValidDriverDeviceValue(temporary) || !temporary.valid)
        return DriverExecutionResult::OUTPUT_VALUE_INVALID;
    output = temporary;
    return DriverExecutionResult::SUCCESS;
}
