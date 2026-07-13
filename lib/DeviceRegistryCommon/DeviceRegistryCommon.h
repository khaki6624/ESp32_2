#ifndef DEVICE_REGISTRY_COMMON_H
#define DEVICE_REGISTRY_COMMON_H

#include <stddef.h>
#include <stdint.h>

constexpr size_t DEVICE_REGISTRY_CAPACITY = 64;

enum class DeviceRegistryResult : uint8_t
{
    SUCCESS = 0,
    INVALID_DEVICE,
    INVALID_ID,
    INVALID_NAME,
    INVALID_BINDING,
    TEMPLATE_NOT_FOUND,
    TEMPLATE_DISABLED,
    TEMPLATE_DRIVER_NOT_SUPPORTED,
    DUPLICATE_ID,
    DUPLICATE_NAME,
    DUPLICATE_BINDING,
    NOT_FOUND,
    SYSTEM_CAPACITY_FULL,
    OUTPUT_BUFFER_INVALID,
    OUTPUT_BUFFER_TOO_SMALL,
    NO_MATCHES
};

inline bool isDeviceRegistrySuccess(DeviceRegistryResult result)
{
    return result == DeviceRegistryResult::SUCCESS;
}

#endif
