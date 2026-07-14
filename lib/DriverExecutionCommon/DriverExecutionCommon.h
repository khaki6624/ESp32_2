#ifndef DRIVER_EXECUTION_COMMON_H
#define DRIVER_EXECUTION_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <math.h>

#include <DeviceCommon.h>

constexpr size_t OUTPUT_DRIVER_BINDING_CAPACITY = 32;
constexpr size_t INPUT_DRIVER_BINDING_CAPACITY = 32;

enum class DriverExecutionResult : uint8_t
{
    SUCCESS = 0,
    INVALID_BINDING,
    BINDING_NOT_FOUND,
    BINDING_ALREADY_REGISTERED,
    PORT_ALREADY_REGISTERED,
    RESOLVER_FULL,
    DEVICE_INVALID,
    DEVICE_DISABLED,
    DEVICE_NOT_CONFIGURED,
    DEVICE_HEALTH_ERROR,
    TEMPLATE_INVALID,
    TEMPLATE_DISABLED,
    TEMPLATE_ACTION_NOT_ALLOWED,
    TEMPLATE_DRIVER_NOT_ALLOWED,
    VALUE_TYPE_MISMATCH,
    ACTION_NOT_SUPPORTED,
    INVALID_ACTION,
    INVALID_VALUE,
    INVALID_DURATION,
    READ_NOT_SUPPORTED,
    WRITE_NOT_SUPPORTED,
    DRIVER_NOT_READY,
    DRIVER_BUSY,
    DRIVER_OFFLINE,
    DRIVER_ERROR,
    REMOTE_BINDING_NOT_SUPPORTED,
    OUTPUT_VALUE_INVALID,
    INPUT_VALUE_INVALID
};

inline bool isValidDriverExecutionResult(DriverExecutionResult result)
{
    switch (result)
    {
        case DriverExecutionResult::SUCCESS:
        case DriverExecutionResult::INVALID_BINDING:
        case DriverExecutionResult::BINDING_NOT_FOUND:
        case DriverExecutionResult::BINDING_ALREADY_REGISTERED:
        case DriverExecutionResult::PORT_ALREADY_REGISTERED:
        case DriverExecutionResult::RESOLVER_FULL:
        case DriverExecutionResult::DEVICE_INVALID:
        case DriverExecutionResult::DEVICE_DISABLED:
        case DriverExecutionResult::DEVICE_NOT_CONFIGURED:
        case DriverExecutionResult::DEVICE_HEALTH_ERROR:
        case DriverExecutionResult::TEMPLATE_INVALID:
        case DriverExecutionResult::TEMPLATE_DISABLED:
        case DriverExecutionResult::TEMPLATE_ACTION_NOT_ALLOWED:
        case DriverExecutionResult::TEMPLATE_DRIVER_NOT_ALLOWED:
        case DriverExecutionResult::VALUE_TYPE_MISMATCH:
        case DriverExecutionResult::ACTION_NOT_SUPPORTED:
        case DriverExecutionResult::INVALID_ACTION:
        case DriverExecutionResult::INVALID_VALUE:
        case DriverExecutionResult::INVALID_DURATION:
        case DriverExecutionResult::READ_NOT_SUPPORTED:
        case DriverExecutionResult::WRITE_NOT_SUPPORTED:
        case DriverExecutionResult::DRIVER_NOT_READY:
        case DriverExecutionResult::DRIVER_BUSY:
        case DriverExecutionResult::DRIVER_OFFLINE:
        case DriverExecutionResult::DRIVER_ERROR:
        case DriverExecutionResult::REMOTE_BINDING_NOT_SUPPORTED:
        case DriverExecutionResult::OUTPUT_VALUE_INVALID:
        case DriverExecutionResult::INPUT_VALUE_INVALID:
            return true;
        default:
            return false;
    }
}

inline bool isDriverExecutionSuccess(DriverExecutionResult result)
{
    return result == DriverExecutionResult::SUCCESS;
}

inline bool isValidDriverDeviceValue(const DeviceValue& value)
{
    if (!value.valid)
        return value.type == DeviceValueType::NONE;
    if (!isValidDeviceValueType(value.type) || value.type == DeviceValueType::NONE)
        return false;

    switch (value.type)
    {
        case DeviceValueType::BOOLEAN:
        case DeviceValueType::INTEGER:
        case DeviceValueType::ENUM_VALUE:
            return true;
        case DeviceValueType::FLOAT:
            return isfinite(value.floatValue);
        case DeviceValueType::PERCENTAGE:
            return value.percentageValue <= 100U;
        case DeviceValueType::NONE:
        default:
            return false;
    }
}

enum class DriverPortDirection : uint8_t
{
    NONE = 0,
    INPUT,
    OUTPUT
};

inline bool isValidDriverPortDirection(DriverPortDirection direction)
{
    switch (direction)
    {
        case DriverPortDirection::NONE:
        case DriverPortDirection::INPUT:
        case DriverPortDirection::OUTPUT:
            return true;
        default:
            return false;
    }
}

enum class DriverPortHealth : uint8_t
{
    UNKNOWN = 0,
    READY,
    BUSY,
    OFFLINE,
    ERROR
};

inline bool isValidDriverPortHealth(DriverPortHealth health)
{
    switch (health)
    {
        case DriverPortHealth::UNKNOWN:
        case DriverPortHealth::READY:
        case DriverPortHealth::BUSY:
        case DriverPortHealth::OFFLINE:
        case DriverPortHealth::ERROR:
            return true;
        default:
            return false;
    }
}

struct DriverActionRequest
{
    DeviceAction action;
    DeviceValue value;
    bool hasValue;
    uint32_t durationMs;
    bool hasDuration;
    uint32_t requestedTimestampMs;

    DriverActionRequest() { clear(); }

    bool isValid() const
    {
        if (!isValidAction(action) || action == DeviceAction::NONE)
            return false;
        if (hasValue ? !value.valid || !isValidDriverDeviceValue(value)
                     : value.valid || value.type != DeviceValueType::NONE)
            return false;
        return hasDuration || durationMs == 0U;
    }

    void clear()
    {
        action = DeviceAction::NONE;
        value.clear();
        hasValue = false;
        durationMs = 0U;
        hasDuration = false;
        requestedTimestampMs = 0U;
    }
};

struct DriverExecutionResponse
{
    DriverExecutionResult result;
    DeviceValue actualValue;
    bool hasActualValue;
    DriverPortHealth portHealth;
    uint32_t completedTimestampMs;

    DriverExecutionResponse() { clear(); }

    bool isValid() const
    {
        if (!isValidDriverExecutionResult(result) || !isValidDriverPortHealth(portHealth))
            return false;
        return hasActualValue
            ? actualValue.valid && isValidDriverDeviceValue(actualValue)
            : !actualValue.valid && actualValue.type == DeviceValueType::NONE;
    }

    void clear()
    {
        result = DriverExecutionResult::DRIVER_NOT_READY;
        actualValue.clear();
        hasActualValue = false;
        portHealth = DriverPortHealth::UNKNOWN;
        completedTimestampMs = 0U;
    }
};

#endif
