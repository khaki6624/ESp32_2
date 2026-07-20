#ifndef NODE_RUNTIME_COMMON_H
#define NODE_RUNTIME_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <DeviceCommon.h>

using NodeMessageId = uint32_t;
using NodeTimestamp = uint32_t;
using NodeDuration = uint32_t;

constexpr NodeMessageId INVALID_NODE_MESSAGE_ID = 0U;
constexpr size_t NODE_RUNTIME_MAX_NODES = 16U;
constexpr size_t NODE_RUNTIME_MAX_ADDRESS_LENGTH = 48U;
constexpr size_t NODE_RUNTIME_MAX_PAYLOAD_LENGTH = 512U;
constexpr NodeDuration NODE_DEFAULT_OFFLINE_TIMEOUT_MS = 60000U;

enum class NodeType : uint8_t
{ MAIN_CONTROLLER = 0, RELAY_NODE, SENSOR_NODE, GARDEN_NODE, WATER_TANK_NODE,
  INDOOR_NODE, SECURITY_NODE, CUSTOM, COUNT };

#ifdef DISABLED
#undef DISABLED
#endif
enum class NodeConnectionState : uint8_t
{ UNKNOWN = 0, OFFLINE, ONLINE, DEGRADED, DISABLED, COUNT };

enum class NodeRuntimeResult : uint8_t
{
    SUCCESS = 0, ACCEPTED, IN_PROGRESS, NO_CHANGE, NOT_INITIALIZED,
    INVALID_ARGUMENT, INVALID_NODE_ID, INVALID_NODE, INVALID_ADDRESS,
    INVALID_MESSAGE, NODE_NOT_FOUND, NODE_DISABLED, NODE_OFFLINE, NODE_BUSY,
    SINK_REJECTED, RETRY_LATER, TIMEOUT, REGISTRY_FULL, DUPLICATE_NODE_ID,
    DUPLICATE_NODE_ADDRESS, REGISTRY_LOCKED, INTERNAL_ERROR, COUNT
};

enum class NodeOutboundSinkResult : uint8_t
{ SUCCESS = 0, ACCEPTED, IN_PROGRESS, RETRY_LATER, REJECTED, FAILED, COUNT };
enum class NodeMessageSinkResult : uint8_t
{ ACCEPTED = 0, REJECTED, RETRY_LATER, COUNT };

inline bool isValidNodeType(NodeType v)
{ return static_cast<uint8_t>(v) < static_cast<uint8_t>(NodeType::COUNT); }
inline bool isValidNodeConnectionState(NodeConnectionState v)
{ return static_cast<uint8_t>(v) < static_cast<uint8_t>(NodeConnectionState::COUNT); }
inline bool isValidNodeRuntimeResult(NodeRuntimeResult v)
{ return static_cast<uint8_t>(v) < static_cast<uint8_t>(NodeRuntimeResult::COUNT); }
inline bool isValidNodeOutboundSinkResult(NodeOutboundSinkResult v)
{ return static_cast<uint8_t>(v) < static_cast<uint8_t>(NodeOutboundSinkResult::COUNT); }
inline bool isValidNodeMessageSinkResult(NodeMessageSinkResult v)
{ return static_cast<uint8_t>(v) < static_cast<uint8_t>(NodeMessageSinkResult::COUNT); }

#endif
