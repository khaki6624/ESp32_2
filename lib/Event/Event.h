#ifndef EVENT_H
#define EVENT_H

#include <string.h>
#include <EventCommon.h>

namespace EventText
{
inline bool isTerminated(const char* value, size_t capacity)
{
    for (size_t i = 0; i < capacity; ++i) if (value[i] == '\0') return true;
    return false;
}

inline bool set(char* destination, size_t capacity, const char* value)
{
    if (value == nullptr) return false;
    size_t length = 0;
    while (length < capacity && value[length] != '\0') ++length;
    if (length >= capacity) return false;
    memset(destination, 0, capacity);
    memcpy(destination, value, length);
    return true;
}
}

struct EventPayload
{
    uint32_t value1;
    uint32_t value2;
    int32_t signedValue;
    bool flag;

    EventPayload() : value1(0), value2(0), signedValue(0), flag(false) {}
};

struct Event
{
    EventId id;
    EventCorrelationId correlationId;
    EventType type;
    EventStatus status;
    EventSeverity severity;
    EventSourceType sourceType;
    uint32_t sourceId;
    uint32_t timestampMs;
    uint32_t processingStartedMs;
    uint32_t completedMs;
    EventPersistencePolicy persistencePolicy;
    EventResultCode resultCode;
    EventPayload payload;
    char sourceName[EVENT_SOURCE_NAME_MAX_LENGTH];
    char message[EVENT_MESSAGE_MAX_LENGTH];

    Event() : id(INVALID_EVENT_ID), correlationId(INVALID_EVENT_CORRELATION_ID),
        type(EventType::NONE), status(EventStatus::PENDING), severity(EventSeverity::INFO),
        sourceType(EventSourceType::NONE), sourceId(0), timestampMs(0),
        processingStartedMs(0), completedMs(0),
        persistencePolicy(EventPersistencePolicy::NONE), resultCode(EventResultCode::NONE), payload{},
        sourceName{}, message{} {}

    bool isValid() const
    {
        return id != INVALID_EVENT_ID && type != EventType::NONE && isValidEventType(type) &&
               isValidEventStatus(status) && isValidEventSeverity(severity) &&
               sourceType != EventSourceType::NONE && isValidEventSourceType(sourceType) &&
               (sourceType == EventSourceType::SYSTEM || sourceId != 0U) &&
               isValidEventPersistencePolicy(persistencePolicy) &&
               isValidEventResultCode(resultCode) &&
               EventText::isTerminated(sourceName, sizeof(sourceName)) &&
               EventText::isTerminated(message, sizeof(message));
    }

    bool isTerminal() const { return isTerminalEventStatus(status); }
    bool setSourceName(const char* value) { return EventText::set(sourceName, sizeof(sourceName), value); }
    bool setMessage(const char* value) { return EventText::set(message, sizeof(message), value); }

    bool transitionTo(EventStatus newStatus, uint32_t transitionTimestampMs,
                      EventResultCode newResultCode = EventResultCode::NONE)
    {
        if (!isValid() || !isValidEventStatus(newStatus) ||
            !isValidEventResultCode(newResultCode) ||
            !isValidEventStatusTransition(status, newStatus)) return false;
        status = newStatus;
        if (newStatus == EventStatus::PROCESSING) processingStartedMs = transitionTimestampMs;
        if (isTerminalEventStatus(newStatus)) completedMs = transitionTimestampMs;
        resultCode = newResultCode;
        return true;
    }
};

#endif
