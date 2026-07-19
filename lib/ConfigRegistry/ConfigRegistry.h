#ifndef CONFIG_REGISTRY_H
#define CONFIG_REGISTRY_H

#include <ConfigDefinition.h>

class ConfigRegistry
{
public:
    ConfigRegistry();
    ConfigResult registerDefinition(const ConfigDefinition& definition);
    const ConfigDefinition* findById(ConfigId id) const;
    const ConfigDefinition* findByKey(const ConfigKey& key) const;
    const ConfigDefinition* getAt(size_t index) const;
    size_t size() const;
    size_t capacity() const;
    bool isFull() const;

private:
    ConfigDefinition entries_[CONFIG_REGISTRY_MAX_ENTRIES];
    size_t count_;
};

#endif
