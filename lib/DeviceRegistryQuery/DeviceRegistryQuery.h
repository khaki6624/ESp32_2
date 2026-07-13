#ifndef DEVICE_REGISTRY_QUERY_H
#define DEVICE_REGISTRY_QUERY_H

#include <stdint.h>

#include <Device.h>

enum class DeviceBooleanFilter : uint8_t
{
    ANY = 0,
    TRUE_ONLY,
    FALSE_ONLY
};

struct DeviceRegistryQuery
{
    NodeId nodeId;
    LocationId locationId;
    DeviceTemplateId templateId;
    DriverType driverType;
    DeviceBooleanFilter enabled;
    DeviceBooleanFilter configured;

    DeviceRegistryQuery() :
        nodeId(INVALID_NODE_ID),
        locationId(INVALID_LOCATION_ID),
        templateId(INVALID_DEVICE_TEMPLATE_ID),
        driverType(DriverType::NONE),
        enabled(DeviceBooleanFilter::ANY),
        configured(DeviceBooleanFilter::ANY)
    {
    }

    bool isValid() const
    {
        return isValidDriverType(driverType) && isValidBooleanFilter(enabled) &&
               isValidBooleanFilter(configured);
    }

    bool matches(const Device& device) const
    {
        if (!isValid())
            return false;
        if (nodeId != INVALID_NODE_ID && device.binding.nodeId != nodeId)
            return false;
        if (locationId != INVALID_LOCATION_ID && device.locationId != locationId)
            return false;
        if (templateId != INVALID_DEVICE_TEMPLATE_ID && device.templateId != templateId)
            return false;
        if (driverType != DriverType::NONE && device.binding.driverType != driverType)
            return false;
        return matchesBoolean(enabled, device.enabled) &&
               matchesBoolean(configured, device.configured);
    }

private:
    static bool isValidBooleanFilter(DeviceBooleanFilter filter)
    {
        return static_cast<uint8_t>(filter) <=
               static_cast<uint8_t>(DeviceBooleanFilter::FALSE_ONLY);
    }

    static bool matchesBoolean(DeviceBooleanFilter filter, bool value)
    {
        switch (filter)
        {
            case DeviceBooleanFilter::ANY:
                return true;
            case DeviceBooleanFilter::TRUE_ONLY:
                return value;
            case DeviceBooleanFilter::FALSE_ONLY:
                return !value;
            default:
                return false;
        }
    }
};

#endif
