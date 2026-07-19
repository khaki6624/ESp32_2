#include "ConfigKey.h"

#include <string.h>

namespace
{
bool isAllowedConfigKeyCharacter(char value)
{
    return (value >= 'A' && value <= 'Z') ||
           (value >= '0' && value <= '9') || value == '_' || value == '.';
}
}

ConfigKey::ConfigKey() : value_{} {}

bool ConfigKey::set(const char* value)
{
    if (value == nullptr) return false;
    size_t candidateLength = 0U;
    while (candidateLength < sizeof(value_) && value[candidateLength] != '\0')
    {
        if (!isAllowedConfigKeyCharacter(value[candidateLength])) return false;
        ++candidateLength;
    }
    if (candidateLength == 0U || candidateLength >= sizeof(value_)) return false;

    char candidate[CONFIG_KEY_MAX_LENGTH] = {};
    memcpy(candidate, value, candidateLength);
    memcpy(value_, candidate, sizeof(value_));
    return true;
}

bool ConfigKey::isValid() const
{
    if (value_[0] == '\0') return false;
    for (size_t index = 0U; index < sizeof(value_); ++index)
    {
        if (value_[index] == '\0') return true;
        if (!isAllowedConfigKeyCharacter(value_[index])) return false;
    }
    return false;
}

const char* ConfigKey::c_str() const { return value_; }

size_t ConfigKey::length() const
{
    size_t result = 0U;
    while (result < sizeof(value_) && value_[result] != '\0') ++result;
    return result;
}

bool ConfigKey::equals(const ConfigKey& other) const
{
    return isValid() && other.isValid() &&
           memcmp(value_, other.value_, sizeof(value_)) == 0;
}
