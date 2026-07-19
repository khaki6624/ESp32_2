#ifndef COMMUNICATION_COMMON_H
#define COMMUNICATION_COMMON_H

#include <stddef.h>
#include <stdint.h>

using CommunicationBackendId = uint16_t;
using CommunicationMessageId = uint32_t;
constexpr CommunicationBackendId INVALID_COMMUNICATION_BACKEND_ID = 0U;
constexpr CommunicationMessageId INVALID_COMMUNICATION_MESSAGE_ID = 0U;
constexpr size_t COMMUNICATION_MAX_BACKENDS = 8U;
constexpr size_t COMMUNICATION_MAX_ADDRESS_LENGTH = 48U;
constexpr size_t COMMUNICATION_MAX_PAYLOAD_LENGTH = 512U;
constexpr size_t COMMUNICATION_MAX_RX_PER_UPDATE = 1U;

enum class CommunicationType : uint8_t
{ SERIAL = 0, SMS, MQTT, MODBUS, HTTP, WEBSOCKET, TCP, UDP, LOCAL, CUSTOM, COUNT };
enum class CommunicationDirection : uint8_t { INBOUND = 0, OUTBOUND, COUNT };
enum class CommunicationTransactionState : uint8_t
{ IDLE = 0, PENDING, RUNNING, SUCCEEDED, FAILED, CANCELLED, COUNT };
enum class CommunicationResult : uint8_t
{
    SUCCESS = 0, ACCEPTED, IN_PROGRESS, NO_MESSAGE, NOT_INITIALIZED,
    INVALID_ARGUMENT, INVALID_BACKEND_ID, INVALID_MESSAGE, INVALID_ADDRESS,
    INVALID_PAYLOAD, BACKEND_NOT_FOUND, BACKEND_NOT_READY, BACKEND_RETRY_LATER,
    BACKEND_FAILED, BUSY, SINK_REJECTED, CANCELLED, INTERNAL_ERROR,
    REGISTRY_FULL, DUPLICATE_BACKEND_ID, REGISTRY_LOCKED, COUNT
};
enum class CommunicationBackendResult : uint8_t
{ SUCCESS = 0, ACCEPTED, IN_PROGRESS, RETRY_LATER, NO_MESSAGE, FAILED, CANCELLED, COUNT };
enum class InboundMessageSinkResult : uint8_t { ACCEPTED = 0, REJECTED, RETRY_LATER, COUNT };

inline bool isValidCommunicationType(CommunicationType value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(CommunicationType::COUNT); }
inline bool isValidCommunicationBackendResult(CommunicationBackendResult value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(CommunicationBackendResult::COUNT); }
inline bool isValidInboundMessageSinkResult(InboundMessageSinkResult value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(InboundMessageSinkResult::COUNT); }

#endif
