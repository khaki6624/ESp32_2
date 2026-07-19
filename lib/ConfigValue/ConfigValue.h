#ifndef CONFIG_VALUE_H
#define CONFIG_VALUE_H

#include <ConfigCommon.h>

class ConfigValue
{
public:
    ConfigValue();

    static ConfigValue fromBool(bool value);
    static ConfigValue fromUInt32(uint32_t value);
    static ConfigValue fromInt32(int32_t value);
    static ConfigValue fromString(const char* value);

    bool isValid() const;
    ConfigValueType type() const;
    bool getBool(bool& value) const;
    bool getUInt32(uint32_t& value) const;
    bool getInt32(int32_t& value) const;
    bool getString(const char*& value) const;
    bool equals(const ConfigValue& other) const;

private:
    ConfigValueType type_;
    bool valid_;
    bool boolValue_;
    uint32_t uint32Value_;
    int32_t int32Value_;
    char stringValue_[CONFIG_STRING_MAX_LENGTH];
};

#endif
