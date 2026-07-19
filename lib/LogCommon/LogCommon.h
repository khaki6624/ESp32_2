#ifndef LOG_COMMON_H
#define LOG_COMMON_H

#include <stddef.h>
#include <stdint.h>

using LogId = uint32_t;
using LogCode = uint16_t;

constexpr LogId INVALID_LOG_ID = 0U;
constexpr LogCode LOG_CODE_NONE = 0U;
constexpr size_t LOG_MESSAGE_MAX_LENGTH = 64U;
constexpr size_t LOG_QUEUE_CAPACITY = 16U;
constexpr size_t LOG_WRITER_CAPACITY = 4U;

enum class LogLevel : uint8_t
{
    TRACE = 0, DEBUG, INFO, NOTICE, WARNING, ERROR, CRITICAL, COUNT
};
inline bool isValidLogLevel(LogLevel value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(LogLevel::COUNT); }

enum class LogCategory : uint8_t
{
    SYSTEM = 0, COMMAND, SECURITY, DEVICE, NODE, STORAGE, TRANSPORT,
    AUTOMATION, CONFIGURATION, DIAGNOSTIC, COUNT
};
inline bool isValidLogCategory(LogCategory value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(LogCategory::COUNT); }

enum class LogSourceType : uint8_t
{
    SYSTEM = 0, COMMAND, DEVICE, ENDPOINT, NODE, TRANSPORT, STORAGE,
    RULE, SCENE, SCHEDULER, SECURITY, USER, COUNT
};
inline bool isValidLogSourceType(LogSourceType value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(LogSourceType::COUNT); }

enum class LogPublishResult : uint8_t
{
    SUCCESS = 0, INVALID_RECORD, QUEUE_FULL, DUPLICATE_LOG_ID, COUNT
};

enum class LogQueueResult : uint8_t { SUCCESS = 0, QUEUE_EMPTY };

enum class LogWriteResult : uint8_t
{
    WRITTEN = 0, IGNORED, RETRY_LATER, FAILED, COUNT
};
inline bool isValidLogWriteResult(LogWriteResult value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(LogWriteResult::COUNT); }

enum class LogWriterRegistrationResult : uint8_t
{
    SUCCESS = 0, DUPLICATE_WRITER, REGISTRY_FULL, WRITER_NOT_FOUND
};

enum class LoggerUpdateResult : uint8_t
{
    SUCCESS = 0, QUEUE_EMPTY, RECORD_IGNORED, NO_WRITERS,
    WRITER_FAILED, RETRY_LATER, INVALID_RECORD, INTERNAL_ERROR, REENTRANT_CALL
};

#endif
