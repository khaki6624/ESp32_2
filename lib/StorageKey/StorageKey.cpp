#include "StorageKey.h"

#include <string.h>

namespace
{
bool isAllowedStorageKeyCharacter(char value)
{
    return (value >= 'A' && value <= 'Z') ||
           (value >= 'a' && value <= 'z') ||
           (value >= '0' && value <= '9') ||
           value == '_' || value == '-' || value == '.' || value == '/';
}
}

StorageKey::StorageKey() : value_{} {}

bool StorageKey::set(const char* value)
{
    if (value == nullptr) return false;
    size_t candidateLength = 0U;
    while (candidateLength < sizeof(value_) && value[candidateLength] != '\0')
    {
        if (!isAllowedStorageKeyCharacter(value[candidateLength])) return false;
        ++candidateLength;
    }
    if (candidateLength == 0U || candidateLength >= sizeof(value_)) return false;
    char candidate[STORAGE_KEY_MAX_LENGTH] = {};
    memcpy(candidate, value, candidateLength);
    memcpy(value_, candidate, sizeof(value_));
    return true;
}

bool StorageKey::isValid() const
{
    if (value_[0] == '\0') return false;
    for (size_t index = 0U; index < sizeof(value_); ++index)
    {
        if (value_[index] == '\0') return true;
        if (!isAllowedStorageKeyCharacter(value_[index])) return false;
    }
    return false;
}

const char* StorageKey::c_str() const { return value_; }

size_t StorageKey::length() const
{
    size_t result = 0U;
    while (result < sizeof(value_) && value_[result] != '\0') ++result;
    return result;
}

bool StorageKey::equals(const StorageKey& other) const
{
    return isValid() && other.isValid() &&
           memcmp(value_, other.value_, sizeof(value_)) == 0;
}
