#ifndef OUTPUT_DRIVER_PORT_H
#define OUTPUT_DRIVER_PORT_H

#include <DriverExecutionCommon.h>

class OutputDriverPort
{
public:
    virtual ~OutputDriverPort() = default;
    virtual DriverType getDriverType() const = 0;
    virtual bool supportsAction(DeviceAction action) const = 0;
    virtual DeviceValueType getValueType() const = 0;
    virtual DriverPortHealth getHealth() const = 0;
    virtual DriverExecutionResult execute(
        const DriverActionRequest& request,
        DriverExecutionResponse& response
    ) = 0;
    virtual DriverExecutionResult readActualValue(DeviceValue& output) const = 0;
};

#endif
