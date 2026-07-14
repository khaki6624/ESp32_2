#ifndef INPUT_DRIVER_PORT_H
#define INPUT_DRIVER_PORT_H

#include <DriverExecutionCommon.h>

class InputDriverPort
{
public:
    virtual ~InputDriverPort() = default;
    virtual DriverType getDriverType() const = 0;
    virtual DeviceValueType getValueType() const = 0;
    virtual DriverPortHealth getHealth() const = 0;
    virtual DriverExecutionResult read(DeviceValue& output) const = 0;
};

#endif
