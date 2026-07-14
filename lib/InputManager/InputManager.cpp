#include "InputManager.h"

namespace
{
    DriverExecutionResult mapInputPortHealth(DriverPortHealth health)
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
}

InputManager::InputManager(const DriverBindingResolver& resolver) : resolver_(resolver) {}

DriverExecutionResult InputManager::read(
    const Device& device,
    const DeviceTemplate& deviceTemplate,
    DeviceValue& output
) const
{
    DriverExecutionResult result = validateDeviceAndTemplate(device, deviceTemplate);
    if (result != DriverExecutionResult::SUCCESS)
        return result;

    InputDriverPort* port = nullptr;
    result = resolver_.resolveInput(device.binding, port);
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
    result = mapInputPortHealth(port->getHealth());
    if (result != DriverExecutionResult::SUCCESS)
        return result;

    DeviceValue temporary;
    result = port->read(temporary);
    if (result != DriverExecutionResult::SUCCESS)
        return result;
    if (!isValidDriverDeviceValue(temporary) || !temporary.valid)
        return DriverExecutionResult::INPUT_VALUE_INVALID;
    if (temporary.type != deviceTemplate.valueType ||
        temporary.type != port->getValueType())
        return DriverExecutionResult::VALUE_TYPE_MISMATCH;

    output = temporary;
    return DriverExecutionResult::SUCCESS;
}

DriverExecutionResult InputManager::validateDeviceAndTemplate(
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
