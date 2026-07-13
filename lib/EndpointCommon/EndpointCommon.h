#ifndef ENDPOINT_COMMON_H
#define ENDPOINT_COMMON_H

#include <stddef.h>
#include <stdint.h>

#include <DeviceCommon.h>

using EndpointId = uint16_t;
constexpr EndpointId INVALID_ENDPOINT_ID = 0;
constexpr size_t ENDPOINT_REGISTRY_CAPACITY = 96;

enum class EndpointDirection : uint8_t
{
    NONE = 0,
    INPUT,
    OUTPUT,
    BIDIRECTIONAL
};

inline bool isValidEndpointDirection(EndpointDirection direction)
{
    return static_cast<uint8_t>(direction) <=
           static_cast<uint8_t>(EndpointDirection::BIDIRECTIONAL);
}

enum class EndpointAvailability : uint8_t
{
    UNKNOWN = 0,
    AVAILABLE,
    UNAVAILABLE,
    OFFLINE,
    ERROR
};

inline bool isValidEndpointAvailability(EndpointAvailability availability)
{
    return static_cast<uint8_t>(availability) <=
           static_cast<uint8_t>(EndpointAvailability::ERROR);
}

enum class EndpointAssignmentState : uint8_t
{
    UNASSIGNED = 0,
    ASSIGNED
};

enum class EndpointBooleanFilter : uint8_t
{
    ANY = 0,
    TRUE_ONLY,
    FALSE_ONLY
};

enum class EndpointAssignmentFilter : uint8_t
{
    ANY = 0,
    ASSIGNED_ONLY,
    UNASSIGNED_ONLY
};

enum class EndpointAvailabilityFilter : uint8_t
{
    ANY = 0,
    UNKNOWN_ONLY,
    AVAILABLE_ONLY,
    UNAVAILABLE_ONLY,
    OFFLINE_ONLY,
    ERROR_ONLY
};

struct HardwareEndpointKey
{
    NodeId nodeId;
    DriverType driverType;
    uint8_t driverInstance;
    uint8_t channel;

    HardwareEndpointKey() :
        nodeId(INVALID_NODE_ID),
        driverType(DriverType::NONE),
        driverInstance(0),
        channel(0)
    {
    }

    bool isValid() const
    {
        return nodeId != INVALID_NODE_ID && driverType != DriverType::NONE &&
               isValidDriverType(driverType);
    }

    bool operator==(const HardwareEndpointKey& other) const
    {
        return nodeId == other.nodeId && driverType == other.driverType &&
               driverInstance == other.driverInstance && channel == other.channel;
    }

    bool operator!=(const HardwareEndpointKey& other) const
    {
        return !(*this == other);
    }
};

enum class EndpointRegistryResult : uint8_t
{
    SUCCESS = 0,
    INVALID_ENDPOINT,
    INVALID_ID,
    INVALID_KEY,
    INVALID_DIRECTION,
    INVALID_VALUE_TYPE,
    INVALID_ACTION_MASK,
    DUPLICATE_ID,
    DUPLICATE_KEY,
    DUPLICATE_DEVICE_ASSIGNMENT,
    NOT_FOUND,
    CAPACITY_FULL,
    ENDPOINT_DISABLED,
    ENDPOINT_UNAVAILABLE,
    ALREADY_ASSIGNED,
    NOT_ASSIGNED,
    INVALID_DEVICE_ID,
    OUTPUT_BUFFER_INVALID,
    OUTPUT_BUFFER_TOO_SMALL,
    NO_MATCHES
};

inline bool isEndpointRegistrySuccess(EndpointRegistryResult result)
{
    return result == EndpointRegistryResult::SUCCESS;
}

#endif
