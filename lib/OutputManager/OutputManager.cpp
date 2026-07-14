#include "OutputManager.h"

namespace
{
    DriverExecutionResult mapPortHealth(DriverPortHealth health)
    {
        switch (health)
        {
            case DriverPortHealth::READY:
                return DriverExecutionResult::SUCCESS;
            case DriverPortHealth::BUSY:
                return DriverExecutionResult::DRIVER_BUSY;
            case DriverPortHealth::OFFLINE:
                return DriverExecutionResult::DRIVER_OFFLINE;
            case DriverPortHealth::ERROR:
                return DriverExecutionResult::DRIVER_ERROR;
            case DriverPortHealth::UNKNOWN:
            default:
                return DriverExecutionResult::DRIVER_NOT_READY;
        }
    }

    bool isOutputAction(DeviceAction action)
    {
        switch (action)
        {
            case DeviceAction::ON:
            case DeviceAction::OFF:
            case DeviceAction::TOGGLE:
            case DeviceAction::PULSE:
            case DeviceAction::TIMED_ON:
            case DeviceAction::OPEN:
            case DeviceAction::CLOSE:
            case DeviceAction::STOP:
            case DeviceAction::SET_POSITION:
            case DeviceAction::SET_LEVEL:
            case DeviceAction::INCREASE:
            case DeviceAction::DECREASE:
            case DeviceAction::ENABLE:
            case DeviceAction::DISABLE:
            case DeviceAction::RESET:
                return true;
            case DeviceAction::NONE:
            case DeviceAction::READ:
            default:
                return false;
        }
    }

    bool actionRequiresValue(DeviceAction action)
    {
        return action == DeviceAction::SET_LEVEL || action == DeviceAction::SET_POSITION;
    }

    bool actionAllowsOptionalValue(DeviceAction action)
    {
        return action == DeviceAction::INCREASE || action == DeviceAction::DECREASE;
    }
}

OutputManager::OutputManager(const DriverBindingResolver& resolver) : resolver_(resolver) {}

DriverExecutionResult OutputManager::execute(
    const Device& device,
    const DeviceTemplate& deviceTemplate,
    const DriverActionRequest& request,
    DriverExecutionResponse& response
) const
{
    DriverExecutionResult result = validateDeviceAndTemplate(device, deviceTemplate);
    if (result != DriverExecutionResult::SUCCESS)
        return result;

    OutputDriverPort* port = nullptr;
    result = resolver_.resolveOutput(device.binding, port);
    if (result != DriverExecutionResult::SUCCESS)
        return result;
    if (port == nullptr || port->getDriverType() != device.binding.driverType)
        return DriverExecutionResult::INVALID_BINDING;

    result = validateAction(device, deviceTemplate, *port, request);
    if (result != DriverExecutionResult::SUCCESS)
        return result;

    DriverExecutionResponse temporary;
    result = port->execute(request, temporary);
    if (result != DriverExecutionResult::SUCCESS)
        return result;
    if (!temporary.isValid() || temporary.result != DriverExecutionResult::SUCCESS)
        return DriverExecutionResult::OUTPUT_VALUE_INVALID;
    if (temporary.hasActualValue &&
        (temporary.actualValue.type != deviceTemplate.valueType ||
         temporary.actualValue.type != port->getValueType()))
        return DriverExecutionResult::OUTPUT_VALUE_INVALID;

    response = temporary;
    return DriverExecutionResult::SUCCESS;
}

DriverExecutionResult OutputManager::readActualValue(
    const Device& device,
    const DeviceTemplate& deviceTemplate,
    DeviceValue& output
) const
{
    DriverExecutionResult result = validateDeviceAndTemplate(device, deviceTemplate);
    if (result != DriverExecutionResult::SUCCESS)
        return result;

    OutputDriverPort* port = nullptr;
    result = resolver_.resolveOutput(device.binding, port);
    if (result != DriverExecutionResult::SUCCESS)
        return result;
    if (port == nullptr || port->getDriverType() != device.binding.driverType)
        return DriverExecutionResult::INVALID_BINDING;
    if (!deviceTemplate.supportsDriver(device.binding.driverType))
        return DriverExecutionResult::TEMPLATE_DRIVER_NOT_ALLOWED;
    if (deviceTemplate.valueType == DeviceValueType::NONE ||
        port->getValueType() == DeviceValueType::NONE ||
        deviceTemplate.valueType != port->getValueType())
        return DriverExecutionResult::VALUE_TYPE_MISMATCH;
    result = mapPortHealth(port->getHealth());
    if (result != DriverExecutionResult::SUCCESS)
        return result;

    DeviceValue temporary;
    result = port->readActualValue(temporary);
    if (result != DriverExecutionResult::SUCCESS)
        return result;
    if (!isValidDriverDeviceValue(temporary) || !temporary.valid)
        return DriverExecutionResult::OUTPUT_VALUE_INVALID;
    if (temporary.type != deviceTemplate.valueType ||
        temporary.type != port->getValueType())
        return DriverExecutionResult::VALUE_TYPE_MISMATCH;

    output = temporary;
    return DriverExecutionResult::SUCCESS;
}

DriverExecutionResult OutputManager::validateDeviceAndTemplate(
    const Device& device,
    const DeviceTemplate& deviceTemplate
) const
{
    if (!device.isValid())
        return DriverExecutionResult::DEVICE_INVALID;
    if (!device.enabled || device.health == DeviceHealth::DISABLED)
        return DriverExecutionResult::DEVICE_DISABLED;
    if (!device.configured)
        return DriverExecutionResult::DEVICE_NOT_CONFIGURED;
    if (device.health == DeviceHealth::OFFLINE)
        return DriverExecutionResult::DRIVER_OFFLINE;
    if (device.health == DeviceHealth::ERROR)
        return DriverExecutionResult::DEVICE_HEALTH_ERROR;
    if (!deviceTemplate.isValid())
        return DriverExecutionResult::TEMPLATE_INVALID;
    if (!deviceTemplate.enabled)
        return DriverExecutionResult::TEMPLATE_DISABLED;
    if (device.templateId != deviceTemplate.id)
        return DriverExecutionResult::TEMPLATE_INVALID;
    if (!device.binding.isValid())
        return DriverExecutionResult::INVALID_BINDING;
    return DriverExecutionResult::SUCCESS;
}

DriverExecutionResult OutputManager::validateAction(
    const Device& device,
    const DeviceTemplate& deviceTemplate,
    const OutputDriverPort& port,
    const DriverActionRequest& request
) const
{
    if (!request.isValid() || !isOutputAction(request.action))
        return request.action == DeviceAction::READ
            ? DriverExecutionResult::READ_NOT_SUPPORTED
            : DriverExecutionResult::INVALID_ACTION;
    if (!deviceTemplate.supportsDriver(device.binding.driverType))
        return DriverExecutionResult::TEMPLATE_DRIVER_NOT_ALLOWED;
    if (!deviceTemplate.supportsAction(request.action))
        return DriverExecutionResult::TEMPLATE_ACTION_NOT_ALLOWED;
    if (!port.supportsAction(request.action))
        return DriverExecutionResult::ACTION_NOT_SUPPORTED;
    if (deviceTemplate.valueType == DeviceValueType::NONE ||
        port.getValueType() == DeviceValueType::NONE ||
        deviceTemplate.valueType != port.getValueType())
        return DriverExecutionResult::VALUE_TYPE_MISMATCH;

    if (actionRequiresValue(request.action))
    {
        if (!request.hasValue)
            return DriverExecutionResult::INVALID_VALUE;
    }
    else if (!actionAllowsOptionalValue(request.action) && request.hasValue)
    {
        return DriverExecutionResult::INVALID_VALUE;
    }
    if (request.hasValue &&
        (request.value.type != deviceTemplate.valueType ||
         request.value.type != port.getValueType()))
        return DriverExecutionResult::VALUE_TYPE_MISMATCH;

    // Core فقط PULSE را زمان‌دار می‌پذیرد؛ Restore و زمان‌بندی مسئول لایه بالاتر است.
    if (request.action == DeviceAction::PULSE)
    {
        if (!request.hasDuration || request.durationMs == 0U)
            return DriverExecutionResult::INVALID_DURATION;
    }
    else if (request.hasDuration)
    {
        return DriverExecutionResult::INVALID_DURATION;
    }

    return mapPortHealth(port.getHealth());
}
