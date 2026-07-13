#ifndef DEVICE_SERIALIZATION_H
#define DEVICE_SERIALIZATION_H

#include <stddef.h>
#include <stdint.h>
#include <type_traits>

#include <Device.h>
#include <DeviceTemplate.h>

constexpr uint16_t DEVICE_RECORD_VERSION = 1;
constexpr uint16_t DEVICE_TEMPLATE_RECORD_VERSION = 1;

inline bool isValidStoredBoolean(uint8_t value)
{
    return value <= 1U;
}

struct alignas(4) DeviceRecord
{
    uint16_t version = 0;
    DeviceId id = INVALID_DEVICE_ID;
    DeviceTemplateId templateId = INVALID_DEVICE_TEMPLATE_ID;
    char name[DEVICE_NAME_MAX_LENGTH] = {};
    LocationId locationId = INVALID_LOCATION_ID;
    NodeId nodeId = INVALID_NODE_ID;
    DriverType driverType = DriverType::NONE;
    uint8_t driverInstance = 0;
    uint8_t channel = 0;
    uint8_t enabled = 0;
    uint8_t configured = 0;
    uint8_t reserved = 0;
    uint32_t checksum = 0;
};

struct alignas(4) DeviceTemplateRecord
{
    uint16_t version = 0;
    DeviceTemplateId id = INVALID_DEVICE_TEMPLATE_ID;
    char name[DEVICE_TEMPLATE_NAME_MAX_LENGTH] = {};
    char icon[DEVICE_TEMPLATE_ICON_MAX_LENGTH] = {};
    DeviceValueType valueType = DeviceValueType::NONE;
    uint8_t reservedBeforeMasks[3] = {};
    DeviceActionMask allowedActions = 0;
    DriverTypeMask allowedDriverTypes = 0;
    uint8_t systemTemplate = 0;
    uint8_t enabled = 0;
    uint8_t reservedBeforeChecksum[2] = {};
    uint32_t checksum = 0;
};

static_assert(std::is_standard_layout<DeviceRecord>::value, "DeviceRecord layout");
static_assert(std::is_trivially_copyable<DeviceRecord>::value, "DeviceRecord copy");
static_assert(alignof(DeviceRecord) == 4U, "DeviceRecord alignment");
static_assert(sizeof(DeviceRecord) == 60U, "DeviceRecord size");
static_assert(offsetof(DeviceRecord, version) == 0U, "DeviceRecord version offset");
static_assert(offsetof(DeviceRecord, name) == 6U, "DeviceRecord name offset");
static_assert(offsetof(DeviceRecord, enabled) == 53U, "DeviceRecord enabled offset");
static_assert(offsetof(DeviceRecord, configured) == 54U, "DeviceRecord configured offset");
static_assert(offsetof(DeviceRecord, checksum) == 56U, "DeviceRecord checksum offset");

static_assert(std::is_standard_layout<DeviceTemplateRecord>::value, "TemplateRecord layout");
static_assert(std::is_trivially_copyable<DeviceTemplateRecord>::value, "TemplateRecord copy");
static_assert(alignof(DeviceTemplateRecord) == 4U, "TemplateRecord alignment");
static_assert(sizeof(DeviceTemplateRecord) == 80U, "TemplateRecord size");
static_assert(offsetof(DeviceTemplateRecord, version) == 0U, "TemplateRecord version offset");
static_assert(offsetof(DeviceTemplateRecord, name) == 4U, "TemplateRecord name offset");
static_assert(offsetof(DeviceTemplateRecord, icon) == 36U, "TemplateRecord icon offset");
static_assert(offsetof(DeviceTemplateRecord, allowedActions) == 64U, "TemplateRecord action offset");
static_assert(offsetof(DeviceTemplateRecord, allowedDriverTypes) == 68U, "TemplateRecord driver offset");
static_assert(offsetof(DeviceTemplateRecord, systemTemplate) == 72U, "TemplateRecord system offset");
static_assert(offsetof(DeviceTemplateRecord, enabled) == 73U, "TemplateRecord enabled offset");
static_assert(offsetof(DeviceTemplateRecord, checksum) == 76U, "TemplateRecord checksum offset");

class DeviceSerialization
{
public:
    static bool serialize(const Device& device, DeviceRecord& record);
    static bool deserialize(const DeviceRecord& record, Device& device);
    static bool validate(const DeviceRecord& record);
    static uint32_t calculateChecksum(const DeviceRecord& record);

    static bool serializeTemplate(
        const DeviceTemplate& deviceTemplate,
        DeviceTemplateRecord& record
    );
    static bool deserializeTemplate(
        const DeviceTemplateRecord& record,
        DeviceTemplate& deviceTemplate
    );
    static bool validateTemplate(const DeviceTemplateRecord& record);
    static uint32_t calculateTemplateChecksum(const DeviceTemplateRecord& record);
};

#endif
