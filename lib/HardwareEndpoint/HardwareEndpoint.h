#ifndef HARDWARE_ENDPOINT_H
#define HARDWARE_ENDPOINT_H

#include <EndpointCommon.h>

struct HardwareEndpoint
{
    EndpointId id;
    HardwareEndpointKey key;
    EndpointDirection direction;
    DeviceValueType valueType;
    DeviceActionMask supportedActions;
    EndpointAvailability availability;
    DeviceId assignedDeviceId;
    bool enabled;

    HardwareEndpoint() :
        id(INVALID_ENDPOINT_ID),
        key(),
        direction(EndpointDirection::NONE),
        valueType(DeviceValueType::NONE),
        supportedActions(0),
        availability(EndpointAvailability::UNKNOWN),
        assignedDeviceId(INVALID_DEVICE_ID),
        enabled(false)
    {
    }

    bool isValid() const
    {
        // enabled، Availability و Assignment بخشی از هویت Endpoint نیستند.
        return id != INVALID_ENDPOINT_ID && key.isValid() &&
               direction != EndpointDirection::NONE &&
               isValidEndpointDirection(direction) &&
               valueType != DeviceValueType::NONE && isValidDeviceValueType(valueType) &&
               isValidActionMask(supportedActions) &&
               isValidEndpointAvailability(availability);
    }

    bool isAssigned() const
    {
        return assignedDeviceId != INVALID_DEVICE_ID;
    }

    bool isOperational() const
    {
        return isValid() && enabled && availability == EndpointAvailability::AVAILABLE;
    }

    EndpointAssignmentState getAssignmentState() const
    {
        return isAssigned()
            ? EndpointAssignmentState::ASSIGNED
            : EndpointAssignmentState::UNASSIGNED;
    }

    bool supportsAction(DeviceAction action) const
    {
        return isValid() && action != DeviceAction::NONE &&
               hasAction(supportedActions, action);
    }
};

#endif
