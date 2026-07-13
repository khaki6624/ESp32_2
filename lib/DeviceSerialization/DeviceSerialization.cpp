#include "DeviceSerialization.h"

#include <stddef.h>
#include <string.h>

namespace
{
    // CRC32 استاندارد IEEE با Polynomial بازتاب‌یافته 0xEDB88320.
    // Reservedها با Zero-Initialization صفر می‌مانند، Validate می‌شوند و در CRC حضور دارند؛
    // استفاده آینده از آن‌ها نیازمند Version جدید یا Migration صریح است.
    uint32_t updateCRC32(uint32_t crc, const void* rawData, size_t length)
    {
        const uint8_t* data = static_cast<const uint8_t*>(rawData);
        for (size_t index = 0; index < length; ++index)
        {
            crc ^= data[index];
            for (uint8_t bit = 0; bit < 8U; ++bit)
            {
                const uint32_t mask = 0U - (crc & 1U);
                crc = (crc >> 1U) ^ (0xEDB88320UL & mask);
            }
        }
        return crc;
    }

    uint32_t updateUint8(uint32_t crc, uint8_t value)
    {
        return updateCRC32(crc, &value, sizeof(value));
    }

    uint32_t updateUint16(uint32_t crc, uint16_t value)
    {
        const uint32_t expandedValue = value;
        const uint8_t bytes[2] = {
            static_cast<uint8_t>(expandedValue & 0xFFU),
            static_cast<uint8_t>((expandedValue >> 8U) & 0xFFU)
        };
        return updateCRC32(crc, bytes, sizeof(bytes));
    }

    uint32_t updateUint32(uint32_t crc, uint32_t value)
    {
        const uint8_t bytes[4] = {
            static_cast<uint8_t>(value & 0xFFU),
            static_cast<uint8_t>((value >> 8U) & 0xFFU),
            static_cast<uint8_t>((value >> 16U) & 0xFFU),
            static_cast<uint8_t>((value >> 24U) & 0xFFU)
        };
        return updateCRC32(crc, bytes, sizeof(bytes));
    }

    bool isCanonicalFixedText(const char* value, size_t capacity, bool allowEmpty)
    {
        if (value == nullptr || capacity == 0U)
            return false;

        size_t terminatorIndex = 0;
        while (terminatorIndex < capacity && value[terminatorIndex] != '\0')
            ++terminatorIndex;

        if (terminatorIndex >= capacity || (!allowEmpty && terminatorIndex == 0U))
            return false;

        for (size_t index = terminatorIndex + 1U; index < capacity; ++index)
        {
            if (value[index] != '\0')
                return false;
        }
        return true;
    }

    bool copyRecordText(char* destination, size_t capacity, const char* source)
    {
        if (destination == nullptr || source == nullptr || capacity == 0U)
            return false;

        const size_t length = strnlen(source, capacity);
        if (length >= capacity)
            return false;

        memset(destination, 0, capacity);
        memcpy(destination, source, length);
        return true;
    }
}

bool DeviceSerialization::serialize(const Device& device, DeviceRecord& record)
{
    if (!device.isValid())
        return false;

    DeviceRecord result{};
    result.version = DEVICE_RECORD_VERSION;
    result.id = device.id;
    result.templateId = device.templateId;
    if (!copyRecordText(result.name, sizeof(result.name), device.name))
        return false;
    result.locationId = device.locationId;
    result.nodeId = device.binding.nodeId;
    result.driverType = device.binding.driverType;
    result.driverInstance = device.binding.driverInstance;
    result.channel = device.binding.channel;
    result.enabled = device.enabled ? 1U : 0U;
    result.configured = device.configured ? 1U : 0U;
    result.checksum = calculateChecksum(result);
    record = result;
    return true;
}

bool DeviceSerialization::deserialize(const DeviceRecord& record, Device& device)
{
    if (!validate(record))
        return false;

    Device result{};
    result.id = record.id;
    result.templateId = record.templateId;
    if (!result.setName(record.name))
        return false;
    result.locationId = record.locationId;
    result.binding.nodeId = record.nodeId;
    result.binding.driverType = record.driverType;
    result.binding.driverInstance = record.driverInstance;
    result.binding.channel = record.channel;
    result.binding.valid = true;
    result.enabled = record.enabled != 0U;
    result.configured = record.configured != 0U;

    // Stateهای Runtime عمداً از Record بازیابی نمی‌شوند.
    result.state.clear();
    result.health = DeviceHealth::UNKNOWN;
    result.lastUpdateMs = 0;
    result.lastChangeMs = 0;

    if (!result.isValid())
        return false;

    device = result;
    return true;
}

bool DeviceSerialization::validate(const DeviceRecord& record)
{
    return record.version == DEVICE_RECORD_VERSION &&
           record.checksum == calculateChecksum(record) &&
           record.id != INVALID_DEVICE_ID &&
           record.templateId != INVALID_DEVICE_TEMPLATE_ID &&
           isCanonicalFixedText(record.name, sizeof(record.name), false) &&
           isValidStoredBoolean(record.enabled) &&
           isValidStoredBoolean(record.configured) &&
           record.reserved == 0U &&
           (record.configured == 0U || record.locationId != INVALID_LOCATION_ID) &&
           record.nodeId != INVALID_NODE_ID &&
           isValidDriverType(record.driverType) &&
           record.driverType != DriverType::NONE;
}

uint32_t DeviceSerialization::calculateChecksum(const DeviceRecord& record)
{
    uint32_t crc = 0xFFFFFFFFUL;
    crc = updateUint16(crc, record.version);
    crc = updateUint16(crc, record.id);
    crc = updateUint16(crc, record.templateId);
    crc = updateCRC32(crc, record.name, sizeof(record.name));
    crc = updateUint16(crc, record.locationId);
    crc = updateUint16(crc, record.nodeId);
    crc = updateUint8(crc, static_cast<uint8_t>(record.driverType));
    crc = updateUint8(crc, record.driverInstance);
    crc = updateUint8(crc, record.channel);
    crc = updateUint8(crc, record.enabled);
    crc = updateUint8(crc, record.configured);
    crc = updateUint8(crc, record.reserved);
    return crc ^ 0xFFFFFFFFUL;
}

bool DeviceSerialization::serializeTemplate(
    const DeviceTemplate& deviceTemplate,
    DeviceTemplateRecord& record
)
{
    if (!deviceTemplate.isValid())
        return false;

    DeviceTemplateRecord result{};
    result.version = DEVICE_TEMPLATE_RECORD_VERSION;
    result.id = deviceTemplate.id;
    if (!copyRecordText(result.name, sizeof(result.name), deviceTemplate.name) ||
        !copyRecordText(result.icon, sizeof(result.icon), deviceTemplate.icon))
    {
        return false;
    }
    result.valueType = deviceTemplate.valueType;
    result.allowedActions = deviceTemplate.allowedActions;
    result.allowedDriverTypes = deviceTemplate.allowedDriverTypes;
    result.systemTemplate = deviceTemplate.systemTemplate ? 1U : 0U;
    result.enabled = deviceTemplate.enabled ? 1U : 0U;
    result.checksum = calculateTemplateChecksum(result);
    record = result;
    return true;
}

bool DeviceSerialization::deserializeTemplate(
    const DeviceTemplateRecord& record,
    DeviceTemplate& deviceTemplate
)
{
    if (!validateTemplate(record))
        return false;

    DeviceTemplate result{};
    result.id = record.id;
    if (!result.setName(record.name) || !result.setIcon(record.icon))
        return false;
    result.valueType = record.valueType;
    result.allowedActions = record.allowedActions;
    result.allowedDriverTypes = record.allowedDriverTypes;
    result.systemTemplate = record.systemTemplate != 0U;
    result.enabled = record.enabled != 0U;

    if (!result.isValid())
        return false;

    deviceTemplate = result;
    return true;
}

bool DeviceSerialization::validateTemplate(const DeviceTemplateRecord& record)
{
    return record.version == DEVICE_TEMPLATE_RECORD_VERSION &&
           record.checksum == calculateTemplateChecksum(record) &&
           record.id != INVALID_DEVICE_TEMPLATE_ID &&
           isCanonicalFixedText(record.name, sizeof(record.name), false) &&
           isCanonicalFixedText(record.icon, sizeof(record.icon), true) &&
           isValidDeviceValueType(record.valueType) &&
           record.valueType != DeviceValueType::NONE &&
           isValidActionMask(record.allowedActions) &&
           record.allowedDriverTypes != 0U &&
           isValidDriverTypeMask(record.allowedDriverTypes) &&
           isValidStoredBoolean(record.systemTemplate) &&
           isValidStoredBoolean(record.enabled) &&
           record.reservedBeforeMasks[0] == 0U &&
           record.reservedBeforeMasks[1] == 0U &&
           record.reservedBeforeMasks[2] == 0U &&
           record.reservedBeforeChecksum[0] == 0U &&
           record.reservedBeforeChecksum[1] == 0U;
}

uint32_t DeviceSerialization::calculateTemplateChecksum(
    const DeviceTemplateRecord& record
)
{
    uint32_t crc = 0xFFFFFFFFUL;
    crc = updateUint16(crc, record.version);
    crc = updateUint16(crc, record.id);
    crc = updateCRC32(crc, record.name, sizeof(record.name));
    crc = updateCRC32(crc, record.icon, sizeof(record.icon));
    crc = updateUint8(crc, static_cast<uint8_t>(record.valueType));
    crc = updateCRC32(crc, record.reservedBeforeMasks, sizeof(record.reservedBeforeMasks));
    crc = updateUint32(crc, record.allowedActions);
    crc = updateUint32(crc, record.allowedDriverTypes);
    crc = updateUint8(crc, record.systemTemplate);
    crc = updateUint8(crc, record.enabled);
    crc = updateCRC32(
        crc,
        record.reservedBeforeChecksum,
        sizeof(record.reservedBeforeChecksum)
    );
    return crc ^ 0xFFFFFFFFUL;
}
