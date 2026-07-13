#ifndef EVENT_HISTORY_RECORD_H
#define EVENT_HISTORY_RECORD_H

#include <type_traits>
#include <Event.h>

constexpr uint16_t EVENT_HISTORY_RECORD_VERSION = 1;

struct EventHistoryRecord
{
    uint16_t version;
    EventId eventId;
    EventCorrelationId correlationId;
    EventType type;
    EventSeverity severity;
    EventSourceType sourceType;
    uint32_t sourceId;
    uint32_t createdTimestampMs;
    uint32_t completedTimestampMs;
    EventStatus terminalStatus;
    EventResultCode resultCode;
    char sourceName[EVENT_SOURCE_NAME_MAX_LENGTH];
    char message[EVENT_MESSAGE_MAX_LENGTH];
    uint32_t checksum;

    EventHistoryRecord() : version(EVENT_HISTORY_RECORD_VERSION), eventId(INVALID_EVENT_ID),
        correlationId(INVALID_EVENT_CORRELATION_ID), type(EventType::NONE),
        severity(EventSeverity::INFO), sourceType(EventSourceType::NONE), sourceId(0),
        createdTimestampMs(0), completedTimestampMs(0), terminalStatus(EventStatus::PENDING),
        resultCode(EventResultCode::NONE), sourceName{}, message{}, checksum(0) {}

    bool isValid() const
    {
        return version == EVENT_HISTORY_RECORD_VERSION && eventId != INVALID_EVENT_ID &&
               type != EventType::NONE && isValidEventType(type) &&
               isValidEventSeverity(severity) && sourceType != EventSourceType::NONE &&
               isValidEventSourceType(sourceType) && isTerminalEventStatus(terminalStatus) &&
               isValidEventResultCode(resultCode) &&
               EventText::isTerminated(sourceName, sizeof(sourceName)) &&
               EventText::isTerminated(message, sizeof(message));
    }
};

static_assert(std::is_standard_layout<EventHistoryRecord>::value, "History must be standard layout");
static_assert(std::is_trivially_copyable<EventHistoryRecord>::value, "History must be trivially copyable");

#endif
