#ifndef EVENT_COMMON_H
#define EVENT_COMMON_H

#include <stddef.h>
#include <stdint.h>

using EventId = uint32_t;
using EventCorrelationId = uint32_t;

constexpr EventId INVALID_EVENT_ID = 0;
constexpr EventCorrelationId INVALID_EVENT_CORRELATION_ID = 0;
constexpr size_t EVENT_BUS_CAPACITY = 32;
constexpr size_t EVENT_QUEUE_CAPACITY = 16;
constexpr size_t EVENT_HANDLER_CAPACITY = 8;
constexpr size_t EVENT_HISTORY_BUFFER_CAPACITY = 16;
constexpr size_t EVENT_SOURCE_NAME_MAX_LENGTH = 24;
constexpr size_t EVENT_MESSAGE_MAX_LENGTH = 64;

// مقادیر صریح و گروه‌بندی‌شده، معنی داده‌های ذخیره‌شده را پایدار نگه می‌دارند.
enum class EventType : uint16_t
{
    NONE = 0,
    SYSTEM_STARTED = 100,
    SYSTEM_REBOOTED = 101,
    SYSTEM_MODE_CHANGED = 102,
    COMMAND_RECEIVED = 200,
    COMMAND_ACCEPTED = 201,
    COMMAND_REJECTED = 202,
    COMMAND_EXECUTED = 203,
    COMMAND_FAILED = 204,
    DEVICE_STATE_CHANGED = 300,
    DEVICE_HEALTH_CHANGED = 301,
    ENDPOINT_ASSIGNED = 400,
    ENDPOINT_UNASSIGNED = 401,
    ENDPOINT_AVAILABILITY_CHANGED = 402,
    NODE_ONLINE = 500,
    NODE_OFFLINE = 501,
    NODE_HEALTH_CHANGED = 502,
    SAFETY_INTERLOCK_TRIGGERED = 600,
    CONFIRMATION_REQUIRED = 601,
    STORAGE_ERROR = 700,
    COMMUNICATION_ERROR = 701,
    CUSTOM = 1000,
    COUNT = 1001
};

inline bool isValidEventType(EventType type)
{
    switch (type)
    {
        case EventType::NONE: case EventType::SYSTEM_STARTED:
        case EventType::SYSTEM_REBOOTED: case EventType::SYSTEM_MODE_CHANGED:
        case EventType::COMMAND_RECEIVED: case EventType::COMMAND_ACCEPTED:
        case EventType::COMMAND_REJECTED: case EventType::COMMAND_EXECUTED:
        case EventType::COMMAND_FAILED: case EventType::DEVICE_STATE_CHANGED:
        case EventType::DEVICE_HEALTH_CHANGED: case EventType::ENDPOINT_ASSIGNED:
        case EventType::ENDPOINT_UNASSIGNED: case EventType::ENDPOINT_AVAILABILITY_CHANGED:
        case EventType::NODE_ONLINE: case EventType::NODE_OFFLINE:
        case EventType::NODE_HEALTH_CHANGED: case EventType::SAFETY_INTERLOCK_TRIGGERED:
        case EventType::CONFIRMATION_REQUIRED: case EventType::STORAGE_ERROR:
        case EventType::COMMUNICATION_ERROR: case EventType::CUSTOM:
            return true;
        default: return false;
    }
}

enum class EventStatus : uint8_t { PENDING = 0, PROCESSING, COMPLETED, FAILED, CANCELLED };
inline bool isValidEventStatus(EventStatus value) { return static_cast<uint8_t>(value) <= 4U; }
inline bool isTerminalEventStatus(EventStatus value)
{
    return value == EventStatus::COMPLETED || value == EventStatus::FAILED ||
           value == EventStatus::CANCELLED;
}

enum class EventSeverity : uint8_t { INFO = 0, NOTICE, WARNING, ERROR, CRITICAL, COUNT };
inline bool isValidEventSeverity(EventSeverity value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(EventSeverity::COUNT); }

enum class EventSourceType : uint8_t
{
    NONE = 0, SYSTEM, COMMAND, DEVICE, ENDPOINT, NODE, TRANSPORT,
    STORAGE, RULE, SCENE, SCHEDULER, USER, COUNT
};
inline bool isValidEventSourceType(EventSourceType value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(EventSourceType::COUNT); }

enum class EventPublishResult : uint8_t
{
    SUCCESS = 0, INVALID_EVENT, QUEUE_FULL, DUPLICATE_EVENT
};

enum class EventQueueResult : uint8_t { SUCCESS = 0, QUEUE_EMPTY };

enum class EventHandleResult : uint8_t { HANDLED = 0, IGNORED, FAILED, COUNT };
inline bool isValidEventHandleResult(EventHandleResult value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(EventHandleResult::COUNT); }

enum class EventHandlerRegistrationResult : uint8_t
{
    SUCCESS = 0, DUPLICATE_HANDLER, REGISTRY_FULL, HANDLER_NOT_FOUND
};

enum class EventDispatchResult : uint8_t
{
    SUCCESS = 0, QUEUE_EMPTY, EVENT_IGNORED, NO_HANDLERS,
    HANDLER_FAILED, INVALID_EVENT, INTERNAL_ERROR, REENTRANT_CALL
};

enum class EventPersistencePolicy : uint8_t { NONE = 0, ON_FAILURE, ALWAYS };
inline bool isValidEventPersistencePolicy(EventPersistencePolicy value)
{
    return static_cast<uint8_t>(value) <= 2U;
}

enum class EventResultCode : uint16_t
{
    NONE = 0, SUCCESS, INVALID_EVENT, INVALID_ID, INVALID_TYPE, INVALID_STATUS,
    QUEUE_FULL, DUPLICATE_ID, NOT_FOUND, INVALID_TRANSITION, NOT_TERMINAL,
    HISTORY_BUFFER_INVALID, HISTORY_BUFFER_FULL
};
inline bool isValidEventResultCode(EventResultCode value)
{
    return static_cast<uint16_t>(value) <= static_cast<uint16_t>(EventResultCode::HISTORY_BUFFER_FULL);
}

inline bool isValidEventStatusTransition(EventStatus from, EventStatus to)
{
    if (!isValidEventStatus(from) || !isValidEventStatus(to)) return false;
    if (from == EventStatus::PENDING)
        return to == EventStatus::PROCESSING || to == EventStatus::FAILED ||
               to == EventStatus::CANCELLED;
    if (from == EventStatus::PROCESSING)
        return to == EventStatus::COMPLETED || to == EventStatus::FAILED ||
               to == EventStatus::CANCELLED;
    return false;
}

#endif
