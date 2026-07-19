#ifndef STORAGE_COMMON_H
#define STORAGE_COMMON_H

#include <stddef.h>
#include <stdint.h>

using StorageOperationId = uint32_t;

constexpr StorageOperationId INVALID_STORAGE_OPERATION_ID = 0U;
constexpr size_t STORAGE_KEY_MAX_LENGTH = 48U;
constexpr size_t STORAGE_MAX_DATA_LENGTH = 1024U;

enum class StorageOperationType : uint8_t
{
    READ = 0, WRITE, REMOVE, EXISTS, COUNT
};

enum class StorageOperationState : uint8_t
{
    IDLE = 0, PENDING, RUNNING, SUCCEEDED, FAILED, CANCELLED, COUNT
};

enum class StorageResult : uint8_t
{
    SUCCESS = 0, ACCEPTED, IN_PROGRESS, NOT_FOUND, ALREADY_EXISTS,
    INVALID_ARGUMENT, INVALID_KEY, INVALID_BUFFER, INVALID_OPERATION,
    NOT_INITIALIZED, BUSY, BACKEND_NOT_READY, BACKEND_RETRY_LATER,
    BACKEND_FAILED, BUFFER_TOO_SMALL, DATA_TOO_LARGE, CANCELLED,
    INTERNAL_ERROR, COUNT
};

enum class StorageBackendResult : uint8_t
{
    SUCCESS = 0, ACCEPTED, IN_PROGRESS, RETRY_LATER, NOT_FOUND,
    ALREADY_EXISTS, BUFFER_TOO_SMALL, FAILED, CANCELLED, COUNT
};

inline bool isValidStorageOperationType(StorageOperationType value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(StorageOperationType::COUNT); }

inline bool isValidStorageBackendResult(StorageBackendResult value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(StorageBackendResult::COUNT); }

#endif
