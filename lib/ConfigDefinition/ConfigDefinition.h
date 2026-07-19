#ifndef CONFIG_DEFINITION_H
#define CONFIG_DEFINITION_H

#include <ConfigKey.h>
#include <ConfigValue.h>

class ConfigDefinition
{
public:
    ConfigDefinition();

    static ConfigDefinition booleanConfig(ConfigId id, const char* key,
        bool defaultValue, ConfigAccess access);
    static ConfigDefinition uint32Config(ConfigId id, const char* key,
        uint32_t defaultValue, uint32_t minimum, uint32_t maximum,
        ConfigAccess access);
    static ConfigDefinition int32Config(ConfigId id, const char* key,
        int32_t defaultValue, int32_t minimum, int32_t maximum,
        ConfigAccess access);
    static ConfigDefinition stringConfig(ConfigId id, const char* key,
        const char* defaultValue, size_t minimumLength, size_t maximumLength,
        bool allowEmpty, ConfigAccess access);

    bool isValid() const;
    ConfigId id() const;
    const ConfigKey& key() const;
    ConfigValueType type() const;
    const ConfigValue& defaultValue() const;
    ConfigAccess access() const;
    uint32_t uint32Minimum() const;
    uint32_t uint32Maximum() const;
    int32_t int32Minimum() const;
    int32_t int32Maximum() const;
    size_t stringMinimumLength() const;
    size_t stringMaximumLength() const;
    bool stringAllowsEmpty() const;

private:
    ConfigId id_;
    ConfigKey key_;
    ConfigValueType type_;
    ConfigValue defaultValue_;
    ConfigAccess access_;
    uint32_t uint32Minimum_;
    uint32_t uint32Maximum_;
    int32_t int32Minimum_;
    int32_t int32Maximum_;
    size_t stringMinimumLength_;
    size_t stringMaximumLength_;
    bool stringAllowsEmpty_;
    bool valid_;
};

#endif
