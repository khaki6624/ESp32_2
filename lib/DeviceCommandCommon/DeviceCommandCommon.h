#ifndef DEVICE_COMMAND_COMMON_H
#define DEVICE_COMMAND_COMMON_H

#include <DriverExecutionCommon.h>

enum class DeviceCommandHandlerResult : uint8_t
{
    SUCCESS = 0,
    INVALID_COMMAND,
    INVALID_DOMAIN,
    INVALID_DOMAIN_INDEX,
    INVALID_OPERATION,
    INVALID_QUERY,
    DEVICE_NOT_FOUND,
    TEMPLATE_NOT_FOUND,
    DEVICE_TEMPLATE_MISMATCH,
    ACTION_MAPPING_FAILED,
    ARGUMENT_MAPPING_FAILED,
    DURATION_NOT_SUPPORTED,
    DRIVER_EXECUTION_FAILED,
    DRIVER_READ_FAILED,
    COMMAND_RESULT_INVALID,
    UNSUPPORTED_COMMAND
};

inline bool isValidDeviceCommandHandlerResult(DeviceCommandHandlerResult result)
{
    switch (result)
    {
        case DeviceCommandHandlerResult::SUCCESS:
        case DeviceCommandHandlerResult::INVALID_COMMAND:
        case DeviceCommandHandlerResult::INVALID_DOMAIN:
        case DeviceCommandHandlerResult::INVALID_DOMAIN_INDEX:
        case DeviceCommandHandlerResult::INVALID_OPERATION:
        case DeviceCommandHandlerResult::INVALID_QUERY:
        case DeviceCommandHandlerResult::DEVICE_NOT_FOUND:
        case DeviceCommandHandlerResult::TEMPLATE_NOT_FOUND:
        case DeviceCommandHandlerResult::DEVICE_TEMPLATE_MISMATCH:
        case DeviceCommandHandlerResult::ACTION_MAPPING_FAILED:
        case DeviceCommandHandlerResult::ARGUMENT_MAPPING_FAILED:
        case DeviceCommandHandlerResult::DURATION_NOT_SUPPORTED:
        case DeviceCommandHandlerResult::DRIVER_EXECUTION_FAILED:
        case DeviceCommandHandlerResult::DRIVER_READ_FAILED:
        case DeviceCommandHandlerResult::COMMAND_RESULT_INVALID:
        case DeviceCommandHandlerResult::UNSUPPORTED_COMMAND:
            return true;
        default:
            return false;
    }
}

inline bool isDeviceCommandHandlerSuccess(DeviceCommandHandlerResult result)
{
    return result == DeviceCommandHandlerResult::SUCCESS;
}

enum class DeviceDomainKind : uint8_t
{
    NONE = 0,
    OUTPUT,
    DIGITAL_INPUT,
    ANALOG_INPUT
};

inline DeviceDomainKind commandDomainToDeviceDomainKind(CommandDomain domain)
{
    switch (domain)
    {
        case CommandDomain::OUT:
            return DeviceDomainKind::OUTPUT;
        case CommandDomain::IN:
            return DeviceDomainKind::DIGITAL_INPUT;
        case CommandDomain::ADC:
            return DeviceDomainKind::ANALOG_INPUT;
        default:
            return DeviceDomainKind::NONE;
    }
}

inline DeviceCommandHandlerResult mapDriverExecutionResult(DriverExecutionResult result)
{
    return result == DriverExecutionResult::SUCCESS
        ? DeviceCommandHandlerResult::SUCCESS
        : DeviceCommandHandlerResult::DRIVER_EXECUTION_FAILED;
}

#endif
