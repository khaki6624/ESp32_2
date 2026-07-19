#ifndef CONFIG_RUNTIME_H
#define CONFIG_RUNTIME_H

#include <ConfigRegistry.h>
#include <ConfigValidator.h>

class ConfigRuntime
{
public:
    explicit ConfigRuntime(const ConfigRegistry& registry);
    ConfigResult begin();
    bool isInitialized() const;
    ConfigResult getById(ConfigId id, ConfigValue& output) const;
    ConfigResult getByKey(const ConfigKey& key, ConfigValue& output) const;
    ConfigResult setById(ConfigId id, const ConfigValue& value);
    ConfigResult setByKey(const ConfigKey& key, const ConfigValue& value);
    ConfigResult resetToDefault(ConfigId id);
    ConfigResult resetAllToDefaults();

private:
    const ConfigRegistry& registry_;
    ConfigValue values_[CONFIG_REGISTRY_MAX_ENTRIES];
    bool writeOnceSet_[CONFIG_REGISTRY_MAX_ENTRIES];
    size_t entryCount_;
    bool initialized_;
    ConfigValidator validator_;

    ConfigResult checkReady() const;
    size_t findIndexById(ConfigId id) const;
    size_t findIndexByKey(const ConfigKey& key) const;
    ConfigResult setAt(size_t index, const ConfigValue& value);
};

#endif
