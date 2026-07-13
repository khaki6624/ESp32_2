#include "DeviceTemplateRegistry.h"

#include <string.h>

namespace
{
    constexpr size_t INVALID_TEMPLATE_INDEX = DEVICE_TEMPLATE_REGISTRY_CAPACITY;
}

DeviceTemplateRegistry::DeviceTemplateRegistry() :
    templates_{},
    count_(0)
{
}

void DeviceTemplateRegistry::clear()
{
    // Reset کامل Registry شامل Templateهای سیستمی نیز می‌شود.
    for (size_t index = 0; index < DEVICE_TEMPLATE_REGISTRY_CAPACITY; ++index)
        templates_[index] = DeviceTemplate{};

    count_ = 0;
}

bool DeviceTemplateRegistry::add(const DeviceTemplate& deviceTemplate)
{
    if (!deviceTemplate.isValid() || isFull() || contains(deviceTemplate.id) ||
        containsName(deviceTemplate.name))
    {
        return false;
    }

    templates_[count_] = deviceTemplate;
    ++count_;
    return true;
}

bool DeviceTemplateRegistry::update(const DeviceTemplate& deviceTemplate)
{
    if (!deviceTemplate.isValid() || deviceTemplate.systemTemplate)
        return false;

    const size_t index = findIndexById(deviceTemplate.id);
    if (index == INVALID_TEMPLATE_INDEX || templates_[index].systemTemplate ||
        nameBelongsToAnotherTemplate(deviceTemplate.name, deviceTemplate.id))
    {
        return false;
    }

    // تمام Validationها پیش از Assignment انجام می‌شوند تا Mutation ناقص رخ ندهد.
    templates_[index] = deviceTemplate;
    return true;
}

bool DeviceTemplateRegistry::remove(DeviceTemplateId id)
{
    const size_t index = findIndexById(id);
    if (index == INVALID_TEMPLATE_INDEX || templates_[index].systemTemplate)
        return false;

    for (size_t current = index + 1U; current < count_; ++current)
        templates_[current - 1U] = templates_[current];

    --count_;
    templates_[count_] = DeviceTemplate{};
    return true;
}

DeviceTemplate* DeviceTemplateRegistry::findById(DeviceTemplateId id)
{
    const size_t index = findIndexById(id);
    return index == INVALID_TEMPLATE_INDEX ? nullptr : &templates_[index];
}

const DeviceTemplate* DeviceTemplateRegistry::findById(DeviceTemplateId id) const
{
    const size_t index = findIndexById(id);
    return index == INVALID_TEMPLATE_INDEX ? nullptr : &templates_[index];
}

DeviceTemplate* DeviceTemplateRegistry::findByName(const char* name)
{
    if (name == nullptr || name[0] == '\0')
        return nullptr;

    for (size_t index = 0; index < count_; ++index)
    {
        if (strcmp(templates_[index].name, name) == 0)
            return &templates_[index];
    }
    return nullptr;
}

const DeviceTemplate* DeviceTemplateRegistry::findByName(const char* name) const
{
    if (name == nullptr || name[0] == '\0')
        return nullptr;

    for (size_t index = 0; index < count_; ++index)
    {
        if (strcmp(templates_[index].name, name) == 0)
            return &templates_[index];
    }
    return nullptr;
}

DeviceTemplate* DeviceTemplateRegistry::getAt(size_t index)
{
    return index < count_ ? &templates_[index] : nullptr;
}

const DeviceTemplate* DeviceTemplateRegistry::getAt(size_t index) const
{
    return index < count_ ? &templates_[index] : nullptr;
}

bool DeviceTemplateRegistry::contains(DeviceTemplateId id) const
{
    return findById(id) != nullptr;
}

bool DeviceTemplateRegistry::containsName(const char* name) const
{
    return findByName(name) != nullptr;
}

bool DeviceTemplateRegistry::supportsAction(
    DeviceTemplateId id,
    DeviceAction action
) const
{
    const DeviceTemplate* deviceTemplate = findById(id);
    return deviceTemplate != nullptr && deviceTemplate->enabled &&
           deviceTemplate->isValid() && action != DeviceAction::NONE &&
           deviceTemplate->supportsAction(action);
}

bool DeviceTemplateRegistry::supportsDriver(
    DeviceTemplateId id,
    DriverType driverType
) const
{
    const DeviceTemplate* deviceTemplate = findById(id);
    return deviceTemplate != nullptr && deviceTemplate->enabled &&
           deviceTemplate->isValid() && driverType != DriverType::NONE &&
           deviceTemplate->supportsDriver(driverType);
}

size_t DeviceTemplateRegistry::size() const
{
    return count_;
}

size_t DeviceTemplateRegistry::capacity() const
{
    return DEVICE_TEMPLATE_REGISTRY_CAPACITY;
}

bool DeviceTemplateRegistry::isFull() const
{
    return count_ >= DEVICE_TEMPLATE_REGISTRY_CAPACITY;
}

bool DeviceTemplateRegistry::isEmpty() const
{
    return count_ == 0U;
}

size_t DeviceTemplateRegistry::findIndexById(DeviceTemplateId id) const
{
    if (id == INVALID_DEVICE_TEMPLATE_ID)
        return INVALID_TEMPLATE_INDEX;

    for (size_t index = 0; index < count_; ++index)
    {
        if (templates_[index].id == id)
            return index;
    }
    return INVALID_TEMPLATE_INDEX;
}

bool DeviceTemplateRegistry::nameBelongsToAnotherTemplate(
    const char* name,
    DeviceTemplateId id
) const
{
    if (name == nullptr || name[0] == '\0')
        return true;

    for (size_t index = 0; index < count_; ++index)
    {
        if (templates_[index].id != id && strcmp(templates_[index].name, name) == 0)
            return true;
    }
    return false;
}
