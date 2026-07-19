#include "ConfigDefinition.h"

namespace
{
bool validBase(ConfigId id, const char* key, ConfigAccess access, ConfigKey& parsedKey)
{
    return id != INVALID_CONFIG_ID && isValidConfigAccess(access) && parsedKey.set(key);
}

size_t configStringLength(const char* value)
{
    if (value == nullptr) return CONFIG_STRING_MAX_LENGTH;
    size_t result = 0U;
    while (result < CONFIG_STRING_MAX_LENGTH && value[result] != '\0') ++result;
    return result;
}
}

ConfigDefinition::ConfigDefinition() : id_(INVALID_CONFIG_ID), key_{},
    type_(ConfigValueType::COUNT), defaultValue_{}, access_(ConfigAccess::COUNT),
    uint32Minimum_(0U), uint32Maximum_(0U), int32Minimum_(0), int32Maximum_(0),
    stringMinimumLength_(0U), stringMaximumLength_(0U), stringAllowsEmpty_(false),
    valid_(false) {}

ConfigDefinition ConfigDefinition::booleanConfig(ConfigId id, const char* key,
    bool defaultValue, ConfigAccess access)
{
    ConfigDefinition result;
    if (!validBase(id, key, access, result.key_)) return result;
    result.id_ = id;
    result.type_ = ConfigValueType::BOOLEAN;
    result.defaultValue_ = ConfigValue::fromBool(defaultValue);
    result.access_ = access;
    result.valid_ = true;
    return result;
}

ConfigDefinition ConfigDefinition::uint32Config(ConfigId id, const char* key,
    uint32_t defaultValue, uint32_t minimum, uint32_t maximum, ConfigAccess access)
{
    ConfigDefinition result;
    if (!validBase(id, key, access, result.key_) || minimum > maximum ||
        defaultValue < minimum || defaultValue > maximum) return result;
    result.id_ = id;
    result.type_ = ConfigValueType::UINT32;
    result.defaultValue_ = ConfigValue::fromUInt32(defaultValue);
    result.access_ = access;
    result.uint32Minimum_ = minimum;
    result.uint32Maximum_ = maximum;
    result.valid_ = true;
    return result;
}

ConfigDefinition ConfigDefinition::int32Config(ConfigId id, const char* key,
    int32_t defaultValue, int32_t minimum, int32_t maximum, ConfigAccess access)
{
    ConfigDefinition result;
    if (!validBase(id, key, access, result.key_) || minimum > maximum ||
        defaultValue < minimum || defaultValue > maximum) return result;
    result.id_ = id;
    result.type_ = ConfigValueType::INT32;
    result.defaultValue_ = ConfigValue::fromInt32(defaultValue);
    result.access_ = access;
    result.int32Minimum_ = minimum;
    result.int32Maximum_ = maximum;
    result.valid_ = true;
    return result;
}

ConfigDefinition ConfigDefinition::stringConfig(ConfigId id, const char* key,
    const char* defaultValue, size_t minimumLength, size_t maximumLength,
    bool allowEmpty, ConfigAccess access)
{
    ConfigDefinition result;
    const size_t defaultLength = configStringLength(defaultValue);
    if (!validBase(id, key, access, result.key_) || minimumLength > maximumLength ||
        maximumLength >= CONFIG_STRING_MAX_LENGTH || defaultLength >= CONFIG_STRING_MAX_LENGTH ||
        defaultLength > maximumLength ||
        (defaultLength == 0U ? !allowEmpty : defaultLength < minimumLength)) return result;
    result.id_ = id;
    result.type_ = ConfigValueType::STRING;
    result.defaultValue_ = ConfigValue::fromString(defaultValue);
    result.access_ = access;
    result.stringMinimumLength_ = minimumLength;
    result.stringMaximumLength_ = maximumLength;
    result.stringAllowsEmpty_ = allowEmpty;
    result.valid_ = true;
    return result;
}

bool ConfigDefinition::isValid() const
{
    if (!valid_ || id_ == INVALID_CONFIG_ID || !key_.isValid() ||
        !isValidConfigValueType(type_) || !isValidConfigAccess(access_) ||
        !defaultValue_.isValid() || defaultValue_.type() != type_) return false;
    if (type_ == ConfigValueType::UINT32)
    {
        uint32_t value = 0U;
        return uint32Minimum_ <= uint32Maximum_ && defaultValue_.getUInt32(value) &&
               value >= uint32Minimum_ && value <= uint32Maximum_;
    }
    if (type_ == ConfigValueType::INT32)
    {
        int32_t value = 0;
        return int32Minimum_ <= int32Maximum_ && defaultValue_.getInt32(value) &&
               value >= int32Minimum_ && value <= int32Maximum_;
    }
    if (type_ == ConfigValueType::STRING)
    {
        const char* value = nullptr;
        if (stringMinimumLength_ > stringMaximumLength_ ||
            stringMaximumLength_ >= CONFIG_STRING_MAX_LENGTH ||
            !defaultValue_.getString(value)) return false;
        const size_t length = configStringLength(value);
        return length < CONFIG_STRING_MAX_LENGTH && length <= stringMaximumLength_ &&
               (length == 0U ? stringAllowsEmpty_ : length >= stringMinimumLength_);
    }
    return type_ == ConfigValueType::BOOLEAN;
}

ConfigId ConfigDefinition::id() const { return id_; }
const ConfigKey& ConfigDefinition::key() const { return key_; }
ConfigValueType ConfigDefinition::type() const { return type_; }
const ConfigValue& ConfigDefinition::defaultValue() const { return defaultValue_; }
ConfigAccess ConfigDefinition::access() const { return access_; }
uint32_t ConfigDefinition::uint32Minimum() const { return uint32Minimum_; }
uint32_t ConfigDefinition::uint32Maximum() const { return uint32Maximum_; }
int32_t ConfigDefinition::int32Minimum() const { return int32Minimum_; }
int32_t ConfigDefinition::int32Maximum() const { return int32Maximum_; }
size_t ConfigDefinition::stringMinimumLength() const { return stringMinimumLength_; }
size_t ConfigDefinition::stringMaximumLength() const { return stringMaximumLength_; }
bool ConfigDefinition::stringAllowsEmpty() const { return stringAllowsEmpty_; }
