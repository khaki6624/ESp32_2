#include "ConfigRegistry.h"

ConfigRegistry::ConfigRegistry() : entries_{}, count_(0U) {}

ConfigResult ConfigRegistry::registerDefinition(const ConfigDefinition& definition)
{
    if (!definition.isValid()) return ConfigResult::INVALID_VALUE;
    for (size_t index = 0U; index < count_; ++index)
    {
        if (entries_[index].id() == definition.id()) return ConfigResult::DUPLICATE_ID;
        if (entries_[index].key().equals(definition.key())) return ConfigResult::DUPLICATE_KEY;
    }
    if (isFull()) return ConfigResult::REGISTRY_FULL;
    entries_[count_] = definition;
    ++count_;
    return ConfigResult::SUCCESS;
}

const ConfigDefinition* ConfigRegistry::findById(ConfigId id) const
{
    if (id == INVALID_CONFIG_ID) return nullptr;
    for (size_t index = 0U; index < count_; ++index)
        if (entries_[index].id() == id) return &entries_[index];
    return nullptr;
}

const ConfigDefinition* ConfigRegistry::findByKey(const ConfigKey& key) const
{
    if (!key.isValid()) return nullptr;
    for (size_t index = 0U; index < count_; ++index)
        if (entries_[index].key().equals(key)) return &entries_[index];
    return nullptr;
}

const ConfigDefinition* ConfigRegistry::getAt(size_t index) const
{ return index < count_ ? &entries_[index] : nullptr; }

size_t ConfigRegistry::size() const { return count_; }
size_t ConfigRegistry::capacity() const { return CONFIG_REGISTRY_MAX_ENTRIES; }
bool ConfigRegistry::isFull() const { return count_ == CONFIG_REGISTRY_MAX_ENTRIES; }
