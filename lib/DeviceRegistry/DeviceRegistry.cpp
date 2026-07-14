#include "DeviceRegistry.h"

#include <CommandCommon.h>
#include <string.h>

namespace
{
    constexpr size_t INVALID_DEVICE_INDEX = DEVICE_REGISTRY_CAPACITY;

    bool hasValidDeviceName(const char* name)
    {
        return name != nullptr && name[0] != '\0' &&
               memchr(name, '\0', DEVICE_NAME_MAX_LENGTH) != nullptr;
    }

    CommandDomain commandDomainForDriver(DriverType driverType)
    {
        switch (driverType)
        {
            case DriverType::RELAY:
            case DriverType::DIMMER:
            case DriverType::ANALOG_OUTPUT:
                return CommandDomain::OUT;
            case DriverType::DIGITAL_INPUT:
                return CommandDomain::IN;
            case DriverType::ANALOG_INPUT:
                return CommandDomain::ADC;
            default:
                return CommandDomain::NONE;
        }
    }

    bool bindingMatchesCommandAddress(
        const DeviceBinding& binding,
        CommandDomain domain,
        uint16_t domainIndex
    )
    {
        return binding.isValid() && domainIndex != 0U &&
               commandDomainForDriver(binding.driverType) == domain &&
               static_cast<uint16_t>(binding.channel) == domainIndex;
    }
}

DeviceRegistry::DeviceRegistry(const DeviceTemplateRegistry& templateRegistry) :
    templateRegistry_(templateRegistry),
    devices_{},
    count_(0)
{
}

void DeviceRegistry::clear()
{
    for (size_t index = 0; index < DEVICE_REGISTRY_CAPACITY; ++index)
        devices_[index] = Device{};

    count_ = 0;
}

DeviceRegistryResult DeviceRegistry::add(const Device& device)
{
    const DeviceRegistryResult validationResult = validateForRegistry(device);
    if (validationResult != DeviceRegistryResult::SUCCESS)
        return validationResult;
    if (isFull())
        return DeviceRegistryResult::SYSTEM_CAPACITY_FULL;
    if (contains(device.id))
        return DeviceRegistryResult::DUPLICATE_ID;
    if (containsName(device.name))
        return DeviceRegistryResult::DUPLICATE_NAME;
    if (containsBinding(device.binding))
        return DeviceRegistryResult::DUPLICATE_BINDING;
    if (commandAddressBelongsToAnotherDevice(device.binding, device.id))
        return DeviceRegistryResult::DUPLICATE_COMMAND_ADDRESS;

    devices_[count_] = device;
    ++count_;
    return DeviceRegistryResult::SUCCESS;
}

DeviceRegistryResult DeviceRegistry::update(const Device& device)
{
    const DeviceRegistryResult validationResult = validateForRegistry(device);
    if (validationResult != DeviceRegistryResult::SUCCESS)
        return validationResult;

    const size_t index = findIndexById(device.id);
    if (index == INVALID_DEVICE_INDEX)
        return DeviceRegistryResult::NOT_FOUND;
    if (nameBelongsToAnotherDevice(device.name, device.id))
        return DeviceRegistryResult::DUPLICATE_NAME;
    if (bindingBelongsToAnotherDevice(device.binding, device.id))
        return DeviceRegistryResult::DUPLICATE_BINDING;
    if (commandAddressBelongsToAnotherDevice(device.binding, device.id))
        return DeviceRegistryResult::DUPLICATE_COMMAND_ADDRESS;

    // همه بررسی‌ها پیش از جایگزینی کامل Device انجام می‌شوند.
    devices_[index] = device;
    return DeviceRegistryResult::SUCCESS;
}

DeviceRegistryResult DeviceRegistry::remove(DeviceId id)
{
    if (id == INVALID_DEVICE_ID)
        return DeviceRegistryResult::INVALID_ID;

    const size_t index = findIndexById(id);
    if (index == INVALID_DEVICE_INDEX)
        return DeviceRegistryResult::NOT_FOUND;

    for (size_t current = index + 1U; current < count_; ++current)
        devices_[current - 1U] = devices_[current];

    --count_;
    devices_[count_] = Device{};
    return DeviceRegistryResult::SUCCESS;
}

Device* DeviceRegistry::findById(DeviceId id)
{
    const size_t index = findIndexById(id);
    return index == INVALID_DEVICE_INDEX ? nullptr : &devices_[index];
}

const Device* DeviceRegistry::findById(DeviceId id) const
{
    const size_t index = findIndexById(id);
    return index == INVALID_DEVICE_INDEX ? nullptr : &devices_[index];
}

const Device* DeviceRegistry::findByCommandAddress(
    CommandDomain domain,
    uint16_t domainIndex
) const
{
    if (domainIndex == 0U ||
        (domain != CommandDomain::OUT && domain != CommandDomain::IN &&
         domain != CommandDomain::ADC))
        return nullptr;

    for (size_t index = 0; index < count_; ++index)
    {
        const Device& device = devices_[index];
        if (device.isValid() && device.binding.isValid() &&
            bindingMatchesCommandAddress(device.binding, domain, domainIndex))
            return &device;
    }
    return nullptr;
}

Device* DeviceRegistry::findByName(const char* name)
{
    if (name == nullptr || name[0] == '\0')
        return nullptr;

    for (size_t index = 0; index < count_; ++index)
    {
        if (strcmp(devices_[index].name, name) == 0)
            return &devices_[index];
    }
    return nullptr;
}

const Device* DeviceRegistry::findByName(const char* name) const
{
    if (name == nullptr || name[0] == '\0')
        return nullptr;

    for (size_t index = 0; index < count_; ++index)
    {
        if (strcmp(devices_[index].name, name) == 0)
            return &devices_[index];
    }
    return nullptr;
}

Device* DeviceRegistry::getAt(size_t index)
{
    return index < count_ ? &devices_[index] : nullptr;
}

const Device* DeviceRegistry::getAt(size_t index) const
{
    return index < count_ ? &devices_[index] : nullptr;
}

bool DeviceRegistry::contains(DeviceId id) const
{
    return findById(id) != nullptr;
}

bool DeviceRegistry::containsName(const char* name) const
{
    return findByName(name) != nullptr;
}

bool DeviceRegistry::containsBinding(const DeviceBinding& binding) const
{
    if (!binding.isValid())
        return false;

    for (size_t index = 0; index < count_; ++index)
    {
        if (devices_[index].binding == binding)
            return true;
    }
    return false;
}

size_t DeviceRegistry::countByNode(NodeId nodeId) const
{
    if (nodeId == INVALID_NODE_ID)
        return 0;

    size_t result = 0;
    for (size_t index = 0; index < count_; ++index)
    {
        if (devices_[index].binding.nodeId == nodeId)
            ++result;
    }
    return result;
}

size_t DeviceRegistry::countByLocation(LocationId locationId) const
{
    size_t result = 0;
    for (size_t index = 0; index < count_; ++index)
    {
        if (devices_[index].locationId == locationId)
            ++result;
    }
    return result;
}

size_t DeviceRegistry::countByTemplate(DeviceTemplateId templateId) const
{
    if (templateId == INVALID_DEVICE_TEMPLATE_ID)
        return 0;

    size_t result = 0;
    for (size_t index = 0; index < count_; ++index)
    {
        if (devices_[index].templateId == templateId)
            ++result;
    }
    return result;
}

DeviceRegistryResult DeviceRegistry::query(
    const DeviceRegistryQuery& queryValue,
    DeviceId* outputIds,
    size_t outputCapacity,
    size_t& outputCount
) const
{
    outputCount = 0;
    if (!queryValue.isValid())
        return DeviceRegistryResult::INVALID_DEVICE;
    if (outputIds == nullptr && outputCapacity > 0U)
        return DeviceRegistryResult::OUTPUT_BUFFER_INVALID;

    size_t requiredCount = 0;
    for (size_t index = 0; index < count_; ++index)
    {
        if (queryValue.matches(devices_[index]))
            ++requiredCount;
    }

    if (requiredCount == 0U)
        return DeviceRegistryResult::NO_MATCHES;

    if (requiredCount > outputCapacity)
    {
        outputCount = requiredCount;
        return DeviceRegistryResult::OUTPUT_BUFFER_TOO_SMALL;
    }

    for (size_t index = 0; index < count_; ++index)
    {
        if (queryValue.matches(devices_[index]))
            outputIds[outputCount++] = devices_[index].id;
    }
    return DeviceRegistryResult::SUCCESS;
}

size_t DeviceRegistry::size() const
{
    return count_;
}

size_t DeviceRegistry::capacity() const
{
    return DEVICE_REGISTRY_CAPACITY;
}

bool DeviceRegistry::isFull() const
{
    return count_ >= DEVICE_REGISTRY_CAPACITY;
}

bool DeviceRegistry::isEmpty() const
{
    return count_ == 0U;
}

size_t DeviceRegistry::findIndexById(DeviceId id) const
{
    if (id == INVALID_DEVICE_ID)
        return INVALID_DEVICE_INDEX;

    for (size_t index = 0; index < count_; ++index)
    {
        if (devices_[index].id == id)
            return index;
    }
    return INVALID_DEVICE_INDEX;
}

bool DeviceRegistry::nameBelongsToAnotherDevice(
    const char* name,
    DeviceId id
) const
{
    if (!hasValidDeviceName(name))
        return true;

    for (size_t index = 0; index < count_; ++index)
    {
        if (devices_[index].id != id && strcmp(devices_[index].name, name) == 0)
            return true;
    }
    return false;
}

bool DeviceRegistry::bindingBelongsToAnotherDevice(
    const DeviceBinding& binding,
    DeviceId id
) const
{
    if (!binding.isValid())
        return true;

    for (size_t index = 0; index < count_; ++index)
    {
        if (devices_[index].id != id && devices_[index].binding == binding)
            return true;
    }
    return false;
}

bool DeviceRegistry::commandAddressBelongsToAnotherDevice(
    const DeviceBinding& binding,
    DeviceId id
) const
{
    const CommandDomain domain = commandDomainForDriver(binding.driverType);
    if (!binding.isValid() || domain == CommandDomain::NONE || binding.channel == 0U)
        return false;
    for (size_t index = 0; index < count_; ++index)
    {
        if (devices_[index].id != id &&
            bindingMatchesCommandAddress(
                devices_[index].binding,
                domain,
                static_cast<uint16_t>(binding.channel)
            ))
            return true;
    }
    return false;
}

DeviceRegistryResult DeviceRegistry::validateForRegistry(const Device& device) const
{
    if (device.id == INVALID_DEVICE_ID)
        return DeviceRegistryResult::INVALID_ID;
    if (!hasValidDeviceName(device.name))
        return DeviceRegistryResult::INVALID_NAME;
    if (!device.binding.isValid())
        return DeviceRegistryResult::INVALID_BINDING;
    if (device.configured && device.locationId == INVALID_LOCATION_ID)
        return DeviceRegistryResult::INVALID_DEVICE;
    if (!device.isValid())
        return DeviceRegistryResult::INVALID_DEVICE;

    const DeviceTemplate* deviceTemplate = templateRegistry_.findById(device.templateId);
    if (deviceTemplate == nullptr)
        return DeviceRegistryResult::TEMPLATE_NOT_FOUND;
    if (!deviceTemplate->isValid())
        return DeviceRegistryResult::INVALID_DEVICE;
    if (!deviceTemplate->enabled)
        return DeviceRegistryResult::TEMPLATE_DISABLED;
    if (!deviceTemplate->supportsDriver(device.binding.driverType))
        return DeviceRegistryResult::TEMPLATE_DRIVER_NOT_SUPPORTED;

    return DeviceRegistryResult::SUCCESS;
}
