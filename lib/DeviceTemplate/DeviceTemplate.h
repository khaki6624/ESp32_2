#ifndef DEVICE_TEMPLATE_H
#define DEVICE_TEMPLATE_H

#include <stddef.h>
#include <string.h>

#include <DeviceCommon.h>

struct DeviceTemplate
{
    DeviceTemplateId id = INVALID_DEVICE_TEMPLATE_ID;
    char name[DEVICE_TEMPLATE_NAME_MAX_LENGTH] = {};
    char icon[DEVICE_TEMPLATE_ICON_MAX_LENGTH] = {};
    DeviceValueType valueType = DeviceValueType::NONE;
    DeviceActionMask allowedActions = 0;
    DriverTypeMask allowedDriverTypes = 0;
    bool systemTemplate = false;
    bool enabled = false;

    bool isValid() const
    {
        // enabled مستقل از اعتبار قرارداد Template است؛ Action خالی برای مدل Passive مجاز است.
        return id != INVALID_DEVICE_TEMPLATE_ID && name[0] != '\0' &&
               valueType != DeviceValueType::NONE && isValidDeviceValueType(valueType) &&
               allowedDriverTypes != 0U && isValidDriverTypeMask(allowedDriverTypes) &&
               isValidActionMask(allowedActions);
    }

    bool supportsAction(DeviceAction action) const
    {
        return isValid() && hasAction(allowedActions, action);
    }

    bool supportsDriver(DriverType type) const
    {
        return isValid() && hasDriverType(allowedDriverTypes, type);
    }

    bool setName(const char* value)
    {
        return copyFixedText(name, sizeof(name), value, false);
    }

    bool setIcon(const char* value)
    {
        return copyFixedText(icon, sizeof(icon), value, true);
    }

private:
    static bool copyFixedText(
        char* destination,
        size_t capacity,
        const char* source,
        bool allowEmpty
    )
    {
        if (destination == nullptr || capacity == 0U || source == nullptr)
            return false;

        const size_t length = strnlen(source, capacity);
        if (length >= capacity || (!allowEmpty && length == 0U))
            return false;

        memset(destination, 0, capacity);
        memcpy(destination, source, length);
        return true;
    }
};

#endif
