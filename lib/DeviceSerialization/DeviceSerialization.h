#ifndef DEVICE_SERIALIZATION_H
#define DEVICE_SERIALIZATION_H

#include <stdint.h>

#include <Device.h>
#include <DeviceTemplate.h>

constexpr uint16_t DEVICE_RECORD_VERSION = 1;
constexpr uint16_t DEVICE_TEMPLATE_RECORD_VERSION = 1;

struct DeviceRecord
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
    bool enabled = false;
    bool configured = false;
    uint32_t checksum = 0;
};

struct DeviceTemplateRecord
{
    uint16_t version = 0;
    DeviceTemplateId id = INVALID_DEVICE_TEMPLATE_ID;
    char name[DEVICE_TEMPLATE_NAME_MAX_LENGTH] = {};
    char icon[DEVICE_TEMPLATE_ICON_MAX_LENGTH] = {};
    DeviceValueType valueType = DeviceValueType::NONE;
    DeviceActionMask allowedActions = 0;
    DriverTypeMask allowedDriverTypes = 0;
    bool systemTemplate = false;
    bool enabled = false;
    uint32_t checksum = 0;
};

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
