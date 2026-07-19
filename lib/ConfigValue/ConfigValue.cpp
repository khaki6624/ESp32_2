#include "ConfigValue.h"

#include <string.h>

ConfigValue::ConfigValue() : type_(ConfigValueType::COUNT), valid_(false),
    boolValue_(false), uint32Value_(0U), int32Value_(0), stringValue_{} {}

ConfigValue ConfigValue::fromBool(bool value)
{
    ConfigValue result;
    result.type_ = ConfigValueType::BOOLEAN;
    result.valid_ = true;
    result.boolValue_ = value;
    return result;
}

ConfigValue ConfigValue::fromUInt32(uint32_t value)
{
    ConfigValue result;
    result.type_ = ConfigValueType::UINT32;
    result.valid_ = true;
    result.uint32Value_ = value;
    return result;
}

ConfigValue ConfigValue::fromInt32(int32_t value)
{
    ConfigValue result;
    result.type_ = ConfigValueType::INT32;
    result.valid_ = true;
    result.int32Value_ = value;
    return result;
}

ConfigValue ConfigValue::fromString(const char* value)
{
    ConfigValue result;
    if (value == nullptr) return result;
    size_t valueLength = 0U;
    while (valueLength < sizeof(result.stringValue_) && value[valueLength] != '\0')
        ++valueLength;
    if (valueLength >= sizeof(result.stringValue_)) return result;
    memcpy(result.stringValue_, value, valueLength);
    result.type_ = ConfigValueType::STRING;
    result.valid_ = true;
    return result;
}

bool ConfigValue::isValid() const
{ return valid_ && isValidConfigValueType(type_); }

ConfigValueType ConfigValue::type() const { return type_; }

bool ConfigValue::getBool(bool& value) const
{
    if (!isValid() || type_ != ConfigValueType::BOOLEAN) return false;
    value = boolValue_;
    return true;
}

bool ConfigValue::getUInt32(uint32_t& value) const
{
    if (!isValid() || type_ != ConfigValueType::UINT32) return false;
    value = uint32Value_;
    return true;
}

bool ConfigValue::getInt32(int32_t& value) const
{
    if (!isValid() || type_ != ConfigValueType::INT32) return false;
    value = int32Value_;
    return true;
}

bool ConfigValue::getString(const char*& value) const
{
    if (!isValid() || type_ != ConfigValueType::STRING) return false;
    value = stringValue_;
    return true;
}

bool ConfigValue::equals(const ConfigValue& other) const
{
    if (!isValid() || !other.isValid() || type_ != other.type_) return false;
    switch (type_)
    {
        case ConfigValueType::BOOLEAN: return boolValue_ == other.boolValue_;
        case ConfigValueType::UINT32: return uint32Value_ == other.uint32Value_;
        case ConfigValueType::INT32: return int32Value_ == other.int32Value_;
        case ConfigValueType::STRING:
            return memcmp(stringValue_, other.stringValue_, sizeof(stringValue_)) == 0;
        default: return false;
    }
}
