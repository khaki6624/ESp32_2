#define private public
#include <DeviceTemplateRegistry.h>
#undef private

static_assert(sizeof(DeviceTemplateRegistry) == 1732U, "Registry size");

namespace
{
    DeviceTemplate makeTemplate(
        DeviceTemplateId id,
        const char* name,
        bool systemTemplate = false,
        bool enabled = true
    )
    {
        DeviceTemplate result{};
        result.id = id;
        result.setName(name);
        result.valueType = DeviceValueType::BOOLEAN;
        addAction(result.allowedActions, DeviceAction::ON);
        addDriverType(result.allowedDriverTypes, DriverType::RELAY);
        result.systemTemplate = systemTemplate;
        result.enabled = enabled;
        return result;
    }
}

bool runDeviceTemplateRegistryTests()
{
    DeviceTemplateRegistry registry;
    if (!registry.isEmpty() || registry.size() != 0U || registry.capacity() != 24U)
        return false;

    const DeviceTemplate user = makeTemplate(1, "Light");
    const DeviceTemplate system = makeTemplate(2, "SYSTEM", true);
    if (!registry.add(user) || !registry.add(system) || registry.size() != 2U)
        return false;
    if (registry.add(DeviceTemplate{}) || registry.add(user) ||
        registry.add(makeTemplate(3, "Light")))
        return false;
    if (!registry.add(makeTemplate(3, "LIGHT")))
        return false;

    if (registry.findById(1) == nullptr || registry.findById(0) != nullptr ||
        registry.findByName("Light") == nullptr || registry.findByName(nullptr) != nullptr ||
        registry.findByName("") != nullptr || registry.getAt(0) == nullptr ||
        registry.getAt(registry.size()) != nullptr)
        return false;

    DeviceTemplate updated = makeTemplate(1, "Lamp");
    if (!registry.update(updated) || registry.findByName("Lamp") == nullptr)
        return false;
    if (registry.update(system) || registry.update(makeTemplate(1, "SYSTEM")) ||
        registry.update(makeTemplate(1, "Lamp", true)))
        return false;
    if (registry.findByName("Lamp") == nullptr || registry.size() != 3U)
        return false;

    if (!registry.supportsAction(1, DeviceAction::ON) ||
        registry.supportsAction(1, DeviceAction::NONE) ||
        !registry.supportsDriver(1, DriverType::RELAY) ||
        registry.supportsDriver(1, DriverType::NONE))
        return false;

    DeviceTemplate disabled = makeTemplate(1, "Lamp", false, false);
    if (!registry.update(disabled) || registry.supportsAction(1, DeviceAction::ON) ||
        registry.supportsDriver(1, DriverType::RELAY))
        return false;
    if (!registry.update(updated))
        return false;

    if (registry.remove(2) || registry.remove(99) || !registry.remove(1))
        return false;
    if (registry.size() != 2U || registry.getAt(0) == nullptr ||
        registry.getAt(0)->id != 2U || registry.getAt(1)->id != 3U ||
        registry.templates_[registry.count_].isValid())
        return false;

    const DeviceTemplateRegistry& constRegistry = registry;
    if (constRegistry.findById(2) == nullptr || constRegistry.findByName("SYSTEM") == nullptr ||
        constRegistry.getAt(0) == nullptr || !constRegistry.contains(2) ||
        !constRegistry.containsName("SYSTEM"))
        return false;

    DeviceTemplateRegistry fullRegistry;
    for (size_t index = 0; index < DEVICE_TEMPLATE_REGISTRY_CAPACITY; ++index)
    {
        char name[2] = {static_cast<char>('A' + index), '\0'};
        if (!fullRegistry.add(makeTemplate(static_cast<DeviceTemplateId>(index + 1U), name)))
            return false;
    }
    if (!fullRegistry.isFull() || fullRegistry.add(makeTemplate(25, "Z2")))
        return false;

    registry.clear();
    if (!registry.isEmpty() || registry.size() != 0U)
        return false;
    for (size_t index = 0; index < DEVICE_TEMPLATE_REGISTRY_CAPACITY; ++index)
    {
        if (registry.templates_[index].isValid())
            return false;
    }

    return true;
}
