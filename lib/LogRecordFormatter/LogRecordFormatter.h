#ifndef LOG_RECORD_FORMATTER_H
#define LOG_RECORD_FORMATTER_H

#include <LogRecord.h>

constexpr size_t SERIAL_LOG_FIXED_CONTENT_LENGTH = 63U;
constexpr size_t SERIAL_LOG_MAX_VALUE_CONTENT_LENGTH = 97U;
constexpr size_t SERIAL_LOG_LINE_MAX_CONTENT_LENGTH =
    SERIAL_LOG_FIXED_CONTENT_LENGTH + SERIAL_LOG_MAX_VALUE_CONTENT_LENGTH +
    (LOG_MESSAGE_MAX_LENGTH - 1U);
constexpr size_t SERIAL_LOG_LINE_MAX_LENGTH = 224U;
static_assert(SERIAL_LOG_LINE_MAX_LENGTH >= SERIAL_LOG_LINE_MAX_CONTENT_LENGTH + 1U,
    "بافر باید برای خط کامل و Null Terminator کافی باشد");

enum class LogFormatResult : uint8_t
{
    SUCCESS = 0, INVALID_RECORD, INVALID_ARGUMENT, BUFFER_TOO_SMALL,
    UNSUPPORTED_MESSAGE, INTERNAL_ERROR, COUNT
};

class LogRecordFormatter
{
public:
    LogFormatResult format(
        const LogRecord& record,
        char* destination,
        size_t capacity,
        size_t& writtenLength
    ) const;

private:
    bool appendLiteral(char* destination, size_t capacity, size_t& position,
        const char* literal) const;
    bool appendUnsigned(char* destination, size_t capacity, size_t& position,
        uint32_t value) const;
    bool appendSigned(char* destination, size_t capacity, size_t& position,
        int32_t value) const;
    bool appendMessage(char* destination, size_t capacity, size_t& position,
        const char* message) const;
    const char* levelName(LogLevel level) const;
    const char* categoryName(LogCategory category) const;
    const char* sourceName(LogSourceType source) const;
};

#endif
