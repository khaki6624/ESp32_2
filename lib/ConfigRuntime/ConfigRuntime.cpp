#include "ConfigRuntime.h"

namespace
{
constexpr size_t INVALID_CONFIG_INDEX = CONFIG_REGISTRY_MAX_ENTRIES;
}

ConfigRuntime::ConfigRuntime(const ConfigRegistry& registry) : registry_(registry),
    values_{}, writeOnceSet_{}, entryCount_(0U), initialized_(false), validator_{} {}

ConfigResult ConfigRuntime::begin()
{
    initialized_ = false;
    const size_t candidateCount = registry_.size();
    if (candidateCount > CONFIG_REGISTRY_MAX_ENTRIES) return ConfigResult::INTERNAL_ERROR;

    for (size_t index = 0U; index < candidateCount; ++index)
    {
        const ConfigDefinition* const definition = registry_.getAt(index);
        if (definition == nullptr || !definition->isValid() ||
            validator_.validate(*definition, definition->defaultValue()) != ConfigResult::SUCCESS)
            return ConfigResult::INTERNAL_ERROR;
    }

    for (size_t index = 0U; index < candidateCount; ++index)
    {
        values_[index] = registry_.getAt(index)->defaultValue();
        writeOnceSet_[index] = false;
    }
    for (size_t index = candidateCount; index < CONFIG_REGISTRY_MAX_ENTRIES; ++index)
    {
        values_[index] = ConfigValue{};
        writeOnceSet_[index] = false;
    }
    entryCount_ = candidateCount;
    initialized_ = true;
    return ConfigResult::SUCCESS;
}

bool ConfigRuntime::isInitialized() const { return initialized_; }

ConfigResult ConfigRuntime::checkReady() const
{
    if (!initialized_) return ConfigResult::NOT_INITIALIZED;
    return registry_.size() == entryCount_
        ? ConfigResult::SUCCESS : ConfigResult::INTERNAL_ERROR;
}

size_t ConfigRuntime::findIndexById(ConfigId id) const
{
    for (size_t index = 0U; index < entryCount_; ++index)
    {
        const ConfigDefinition* const definition = registry_.getAt(index);
        if (definition != nullptr && definition->id() == id) return index;
    }
    return INVALID_CONFIG_INDEX;
}

size_t ConfigRuntime::findIndexByKey(const ConfigKey& key) const
{
    for (size_t index = 0U; index < entryCount_; ++index)
    {
        const ConfigDefinition* const definition = registry_.getAt(index);
        if (definition != nullptr && definition->key().equals(key)) return index;
    }
    return INVALID_CONFIG_INDEX;
}

ConfigResult ConfigRuntime::getById(ConfigId id, ConfigValue& output) const
{
    const ConfigResult ready = checkReady();
    if (ready != ConfigResult::SUCCESS) return ready;
    if (id == INVALID_CONFIG_ID) return ConfigResult::INVALID_ID;
    const size_t index = findIndexById(id);
    if (index == INVALID_CONFIG_INDEX) return ConfigResult::NOT_FOUND;
    output = values_[index];
    return ConfigResult::SUCCESS;
}

ConfigResult ConfigRuntime::getByKey(const ConfigKey& key, ConfigValue& output) const
{
    const ConfigResult ready = checkReady();
    if (ready != ConfigResult::SUCCESS) return ready;
    if (!key.isValid()) return ConfigResult::INVALID_KEY;
    const size_t index = findIndexByKey(key);
    if (index == INVALID_CONFIG_INDEX) return ConfigResult::NOT_FOUND;
    output = values_[index];
    return ConfigResult::SUCCESS;
}

ConfigResult ConfigRuntime::setAt(size_t index, const ConfigValue& value)
{
    const ConfigDefinition* const definition = registry_.getAt(index);
    if (definition == nullptr) return ConfigResult::INTERNAL_ERROR;
    if (definition->access() == ConfigAccess::READ_ONLY) return ConfigResult::READ_ONLY;
    if (definition->access() == ConfigAccess::WRITE_ONCE && writeOnceSet_[index])
        return ConfigResult::ALREADY_SET;
    const ConfigResult validation = validator_.validate(*definition, value);
    if (validation != ConfigResult::SUCCESS) return validation;
    values_[index] = value;
    if (definition->access() == ConfigAccess::WRITE_ONCE) writeOnceSet_[index] = true;
    return ConfigResult::SUCCESS;
}

ConfigResult ConfigRuntime::setById(ConfigId id, const ConfigValue& value)
{
    const ConfigResult ready = checkReady();
    if (ready != ConfigResult::SUCCESS) return ready;
    if (id == INVALID_CONFIG_ID) return ConfigResult::INVALID_ID;
    const size_t index = findIndexById(id);
    return index == INVALID_CONFIG_INDEX ? ConfigResult::NOT_FOUND : setAt(index, value);
}

ConfigResult ConfigRuntime::setByKey(const ConfigKey& key, const ConfigValue& value)
{
    const ConfigResult ready = checkReady();
    if (ready != ConfigResult::SUCCESS) return ready;
    if (!key.isValid()) return ConfigResult::INVALID_KEY;
    const size_t index = findIndexByKey(key);
    return index == INVALID_CONFIG_INDEX ? ConfigResult::NOT_FOUND : setAt(index, value);
}

ConfigResult ConfigRuntime::resetToDefault(ConfigId id)
{
    const ConfigResult ready = checkReady();
    if (ready != ConfigResult::SUCCESS) return ready;
    if (id == INVALID_CONFIG_ID) return ConfigResult::INVALID_ID;
    const size_t index = findIndexById(id);
    if (index == INVALID_CONFIG_INDEX) return ConfigResult::NOT_FOUND;
    const ConfigDefinition* const definition = registry_.getAt(index);
    if (definition == nullptr) return ConfigResult::INTERNAL_ERROR;
    values_[index] = definition->defaultValue();
    writeOnceSet_[index] = false;
    return ConfigResult::SUCCESS;
}

ConfigResult ConfigRuntime::resetAllToDefaults()
{
    const ConfigResult ready = checkReady();
    if (ready != ConfigResult::SUCCESS) return ready;
    for (size_t index = 0U; index < entryCount_; ++index)
    {
        const ConfigDefinition* const definition = registry_.getAt(index);
        if (definition == nullptr) return ConfigResult::INTERNAL_ERROR;
    }
    for (size_t index = 0U; index < entryCount_; ++index)
    {
        values_[index] = registry_.getAt(index)->defaultValue();
        writeOnceSet_[index] = false;
    }
    return ConfigResult::SUCCESS;
}
