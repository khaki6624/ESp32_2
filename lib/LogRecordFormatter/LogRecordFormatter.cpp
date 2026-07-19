#include "LogRecordFormatter.h"

namespace
{
void clearFailure(char* destination, size_t capacity, size_t& writtenLength)
{
    writtenLength = 0U;
    if (destination != nullptr && capacity > 0U) destination[0] = '\0';
}
}

LogFormatResult LogRecordFormatter::format(
    const LogRecord& record, char* destination, size_t capacity, size_t& writtenLength
) const
{
    clearFailure(destination, capacity, writtenLength);
    if (destination == nullptr || capacity == 0U) return LogFormatResult::INVALID_ARGUMENT;
    if (!record.isValid()) return LogFormatResult::INVALID_RECORD;

    const char* const level = levelName(record.level);
    const char* const category = categoryName(record.category);
    const char* const source = sourceName(record.sourceType);
    if (level == nullptr || category == nullptr || source == nullptr)
        return LogFormatResult::INVALID_RECORD;

    for (size_t index = 0U; index < sizeof(record.message); ++index)
    {
        const char value = record.message[index];
        if (value == '\0') break;
        if (value == '|' || value == '\r' || value == '\n')
            return LogFormatResult::UNSUPPORTED_MESSAGE;
    }

    size_t position = 0U;
    const bool success =
        appendLiteral(destination, capacity, position, "LOG|id=") &&
        appendUnsigned(destination, capacity, position, record.id) &&
        appendLiteral(destination, capacity, position, "|ts=") &&
        appendUnsigned(destination, capacity, position, record.timestampMs) &&
        appendLiteral(destination, capacity, position, "|level=") &&
        appendLiteral(destination, capacity, position, level) &&
        appendLiteral(destination, capacity, position, "|cat=") &&
        appendLiteral(destination, capacity, position, category) &&
        appendLiteral(destination, capacity, position, "|src=") &&
        appendLiteral(destination, capacity, position, source) &&
        appendLiteral(destination, capacity, position, "|sid=") &&
        appendUnsigned(destination, capacity, position, record.sourceId) &&
        appendLiteral(destination, capacity, position, "|code=") &&
        appendUnsigned(destination, capacity, position, record.code) &&
        appendLiteral(destination, capacity, position, "|v1=") &&
        appendUnsigned(destination, capacity, position, record.payload.value1) &&
        appendLiteral(destination, capacity, position, "|v2=") &&
        appendUnsigned(destination, capacity, position, record.payload.value2) &&
        appendLiteral(destination, capacity, position, "|sv=") &&
        appendSigned(destination, capacity, position, record.payload.signedValue) &&
        appendLiteral(destination, capacity, position, "|flag=") &&
        appendLiteral(destination, capacity, position, record.payload.flag ? "1" : "0") &&
        appendLiteral(destination, capacity, position, "|msg=") &&
        appendMessage(destination, capacity, position, record.message) &&
        appendLiteral(destination, capacity, position, "\n");

    if (!success)
    {
        clearFailure(destination, capacity, writtenLength);
        return LogFormatResult::BUFFER_TOO_SMALL;
    }
    destination[position] = '\0';
    writtenLength = position;
    return LogFormatResult::SUCCESS;
}

bool LogRecordFormatter::appendLiteral(
    char* destination, size_t capacity, size_t& position, const char* literal
) const
{
    if (literal == nullptr) return false;
    size_t length = 0U;
    while (literal[length] != '\0') ++length;
    if (position + length >= capacity) return false;
    for (size_t index = 0U; index < length; ++index)
        destination[position + index] = literal[index];
    position += length;
    return true;
}

bool LogRecordFormatter::appendUnsigned(
    char* destination, size_t capacity, size_t& position, uint32_t value
) const
{
    char reversed[10];
    size_t count = 0U;
    do
    {
        reversed[count++] = static_cast<char>('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U);
    if (position + count >= capacity) return false;
    while (count > 0U) destination[position++] = reversed[--count];
    return true;
}

bool LogRecordFormatter::appendSigned(
    char* destination, size_t capacity, size_t& position, int32_t value
) const
{
    if (value >= 0) return appendUnsigned(destination, capacity, position,
        static_cast<uint32_t>(value));
    if (!appendLiteral(destination, capacity, position, "-")) return false;
    const uint32_t magnitude = static_cast<uint32_t>(-(value + 1)) + 1U;
    return appendUnsigned(destination, capacity, position, magnitude);
}

bool LogRecordFormatter::appendMessage(
    char* destination, size_t capacity, size_t& position, const char* message
) const
{
    return appendLiteral(destination, capacity, position, message);
}

const char* LogRecordFormatter::levelName(LogLevel level) const
{
    switch (level)
    {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::NOTICE: return "NOTICE";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        case LogLevel::COUNT:
        default: return nullptr;
    }
}

const char* LogRecordFormatter::categoryName(LogCategory category) const
{
    switch (category)
    {
        case LogCategory::SYSTEM: return "SYSTEM";
        case LogCategory::COMMAND: return "COMMAND";
        case LogCategory::SECURITY: return "SECURITY";
        case LogCategory::DEVICE: return "DEVICE";
        case LogCategory::NODE: return "NODE";
        case LogCategory::STORAGE: return "STORAGE";
        case LogCategory::TRANSPORT: return "TRANSPORT";
        case LogCategory::AUTOMATION: return "AUTOMATION";
        case LogCategory::CONFIGURATION: return "CONFIGURATION";
        case LogCategory::DIAGNOSTIC: return "DIAGNOSTIC";
        case LogCategory::COUNT:
        default: return nullptr;
    }
}

const char* LogRecordFormatter::sourceName(LogSourceType source) const
{
    switch (source)
    {
        case LogSourceType::SYSTEM: return "SYSTEM";
        case LogSourceType::COMMAND: return "COMMAND";
        case LogSourceType::DEVICE: return "DEVICE";
        case LogSourceType::ENDPOINT: return "ENDPOINT";
        case LogSourceType::NODE: return "NODE";
        case LogSourceType::TRANSPORT: return "TRANSPORT";
        case LogSourceType::STORAGE: return "STORAGE";
        case LogSourceType::RULE: return "RULE";
        case LogSourceType::SCENE: return "SCENE";
        case LogSourceType::SCHEDULER: return "SCHEDULER";
        case LogSourceType::SECURITY: return "SECURITY";
        case LogSourceType::USER: return "USER";
        case LogSourceType::COUNT:
        default: return nullptr;
    }
}
