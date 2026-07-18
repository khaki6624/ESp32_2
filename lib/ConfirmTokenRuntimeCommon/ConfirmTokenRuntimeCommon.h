#ifndef CONFIRM_TOKEN_RUNTIME_COMMON_H
#define CONFIRM_TOKEN_RUNTIME_COMMON_H

#include <stdint.h>

using ConfirmTokenId = uint16_t;
constexpr ConfirmTokenId INVALID_CONFIRM_TOKEN_ID = 0U;
constexpr uint32_t CONFIRM_TOKEN_MAX_TTL_MS = 0x7FFFFFFFUL;

enum class ConfirmTokenStoreResult : uint8_t
{
    SUCCESS = 0,
    INVALID_TOKEN,
    DUPLICATE_VALUE,
    STORE_FULL,
    TOKEN_NOT_FOUND
};

enum class ConfirmTokenIssueResult : uint8_t
{
    SUCCESS = 0,
    INVALID_COMMAND,
    INVALID_TTL,
    GENERATION_FAILED,
    INVALID_GENERATED_VALUE,
    DUPLICATE_VALUE,
    STORE_FULL,
    STORE_ERROR
};

#endif
