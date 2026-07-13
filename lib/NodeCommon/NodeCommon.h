#ifndef NODE_COMMON_H
#define NODE_COMMON_H

#include <stddef.h>
#include <stdint.h>

#include <DeviceCommon.h>

constexpr size_t NODE_REGISTRY_CAPACITY = 16;
constexpr size_t NODE_NAME_MAX_LENGTH = 40;

enum class NodeRole : uint8_t
{
    NONE = 0,
    CENTRAL,
    YARD,
    UTILITY,
    INDOOR,
    GENERIC
};

inline bool isValidNodeRole(NodeRole role)
{
    return static_cast<uint8_t>(role) <= static_cast<uint8_t>(NodeRole::GENERIC);
}

enum class NodeLinkType : uint8_t
{
    NONE = 0,
    LOCAL,
    RS485_MODBUS,
    WIFI,
    ETHERNET,
    CAN_BUS,
    LORA
};

inline bool isValidNodeLinkType(NodeLinkType type)
{
    return static_cast<uint8_t>(type) <= static_cast<uint8_t>(NodeLinkType::LORA);
}

// Arduino Core ممکن است این نام قراردادی را به‌صورت Macro تعریف کرده باشد.
#ifdef DISABLED
#undef DISABLED
#endif

enum class NodeHealthState : uint8_t
{
    UNKNOWN = 0,
    INITIALIZING,
    ONLINE,
    DEGRADED,
    OFFLINE,
    ERROR,
    DISABLED
};

inline bool isValidNodeHealthState(NodeHealthState state)
{
    return static_cast<uint8_t>(state) <= static_cast<uint8_t>(NodeHealthState::DISABLED);
}

enum class NodeBooleanFilter : uint8_t
{
    ANY = 0,
    TRUE_ONLY,
    FALSE_ONLY
};

inline bool isValidNodeBooleanFilter(NodeBooleanFilter filter)
{
    return static_cast<uint8_t>(filter) <= static_cast<uint8_t>(NodeBooleanFilter::FALSE_ONLY);
}

enum class NodeHealthFilter : uint8_t
{
    ANY = 0,
    UNKNOWN_ONLY,
    INITIALIZING_ONLY,
    ONLINE_ONLY,
    DEGRADED_ONLY,
    OFFLINE_ONLY,
    ERROR_ONLY,
    DISABLED_ONLY
};

inline bool isValidNodeHealthFilter(NodeHealthFilter filter)
{
    return static_cast<uint8_t>(filter) <= static_cast<uint8_t>(NodeHealthFilter::DISABLED_ONLY);
}

enum class NodeRegistryResult : uint8_t
{
    SUCCESS = 0,
    INVALID_NODE,
    INVALID_ID,
    INVALID_NAME,
    INVALID_ROLE,
    INVALID_LINK_TYPE,
    INVALID_HEALTH,
    DUPLICATE_ID,
    DUPLICATE_NAME,
    NOT_FOUND,
    CAPACITY_FULL,
    OUTPUT_BUFFER_INVALID,
    OUTPUT_BUFFER_TOO_SMALL,
    NO_MATCHES
};

inline bool isNodeRegistrySuccess(NodeRegistryResult result)
{
    return result == NodeRegistryResult::SUCCESS;
}

#endif
