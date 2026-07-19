#ifndef LOG_RECORD_H
#define LOG_RECORD_H

#include <string.h>
#include <LogCommon.h>

namespace LogText
{
inline bool isTerminated(const char* value, size_t capacity)
{
    for (size_t index = 0U; index < capacity; ++index)
        if (value[index] == '\0') return true;
    return false;
}

inline bool set(char* destination, size_t capacity, const char* value)
{
    if (value == nullptr) return false;
    size_t length = 0U;
    while (length < capacity && value[length] != '\0') ++length;
    if (length >= capacity) return false;
    memset(destination, 0, capacity);
    memcpy(destination, value, length);
    return true;
}
}

struct LogPayload
{
    uint32_t value1;
    uint32_t value2;
    int32_t signedValue;
    bool flag;

    LogPayload() : value1(0U), value2(0U), signedValue(0), flag(false) {}
};

struct LogRecord
{
    LogId id;
    uint32_t timestampMs;
    LogLevel level;
    LogCategory category;
    LogSourceType sourceType;
    uint32_t sourceId;
    LogCode code;
    LogPayload payload;
    char message[LOG_MESSAGE_MAX_LENGTH];

    LogRecord() : id(INVALID_LOG_ID), timestampMs(0U), level(LogLevel::INFO),
        category(LogCategory::SYSTEM), sourceType(LogSourceType::SYSTEM), sourceId(0U),
        code(LOG_CODE_NONE), payload{}, message{} {}

    bool isValid() const
    {
        return id != INVALID_LOG_ID && isValidLogLevel(level) &&
               isValidLogCategory(category) && isValidLogSourceType(sourceType) &&
               (sourceType == LogSourceType::SYSTEM || sourceId != 0U) &&
               LogText::isTerminated(message, sizeof(message));
    }

    bool setMessage(const char* value)
    { return LogText::set(message, sizeof(message), value); }
};

#endif
