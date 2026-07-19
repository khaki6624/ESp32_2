#ifndef CONFIG_COMMON_H
#define CONFIG_COMMON_H

#include <stddef.h>
#include <stdint.h>

using ConfigId = uint16_t;

constexpr ConfigId INVALID_CONFIG_ID = 0U;
constexpr size_t CONFIG_KEY_MAX_LENGTH = 48U;
constexpr size_t CONFIG_STRING_MAX_LENGTH = 64U;
constexpr size_t CONFIG_REGISTRY_MAX_ENTRIES = 64U;

enum class ConfigValueType : uint8_t
{
    BOOLEAN = 0, UINT32, INT32, STRING, COUNT
};

enum class ConfigAccess : uint8_t
{
    READ_ONLY = 0, READ_WRITE, WRITE_ONCE, COUNT
};

enum class ConfigResult : uint8_t
{
    SUCCESS = 0, NOT_FOUND, INVALID_ID, INVALID_KEY, INVALID_VALUE,
    TYPE_MISMATCH, READ_ONLY, ALREADY_SET, REGISTRY_FULL, DUPLICATE_ID,
    DUPLICATE_KEY, NOT_INITIALIZED, INTERNAL_ERROR, COUNT
};

inline bool isValidConfigValueType(ConfigValueType value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(ConfigValueType::COUNT); }

inline bool isValidConfigAccess(ConfigAccess value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(ConfigAccess::COUNT); }

#endif
