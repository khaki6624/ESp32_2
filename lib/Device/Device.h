#ifndef DEVICE_H
#define DEVICE_H

#include <stddef.h>
#include <string.h>

#include <DeviceCommon.h>

struct Device
{
    DeviceId id = INVALID_DEVICE_ID;
    DeviceTemplateId templateId = INVALID_DEVICE_TEMPLATE_ID;
    char name[DEVICE_NAME_MAX_LENGTH] = {};
    LocationId locationId = INVALID_LOCATION_ID;
    DeviceBinding binding{};
    DeviceValue state{};
    DeviceHealth health = DeviceHealth::UNKNOWN;
    bool enabled = false;
    bool configured = false;
    uint32_t createdAt = 0;
    uint32_t lastUpdateMs = 0;
    uint32_t lastChangeMs = 0;

    bool isValid() const
    {
        // enabled و configured بخشی از هویت معتبر Device نیستند.
        return id != INVALID_DEVICE_ID && templateId != INVALID_DEVICE_TEMPLATE_ID &&
               name[0] != '\0' && locationId != INVALID_LOCATION_ID && binding.isValid();
    }

    bool isOperational() const
    {
        return isValid() && enabled && configured &&
               (health == DeviceHealth::OK || health == DeviceHealth::DEGRADED);
    }

    bool setName(const char* value)
    {
        if (value == nullptr)
            return false;

        const size_t length = strnlen(value, sizeof(name));
        if (length == 0U || length >= sizeof(name))
            return false;

        memset(name, 0, sizeof(name));
        memcpy(name, value, length);
        return true;
    }

    bool setState(const DeviceValue& value, uint32_t timestampMs)
    {
        const bool changed = !state.equals(value);
        state = value;
        state.timestampMs = timestampMs;
        lastUpdateMs = timestampMs;
        if (changed)
            lastChangeMs = timestampMs;
        return changed;
    }

    void markUpdated(uint32_t timestampMs)
    {
        lastUpdateMs = timestampMs;
        state.timestampMs = timestampMs;
    }

    void setHealth(DeviceHealth value)
    {
        health = value;
    }
};

#endif
