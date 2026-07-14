#ifndef OUTPUT_MANAGER_H
#define OUTPUT_MANAGER_H

#include <Device.h>
#include <DeviceTemplate.h>
#include <DriverBindingResolver.h>

class OutputManager
{
public:
    explicit OutputManager(const DriverBindingResolver& resolver);

    DriverExecutionResult execute(
        const Device& device,
        const DeviceTemplate& deviceTemplate,
        const DriverActionRequest& request,
        DriverExecutionResponse& response
    ) const;

    DriverExecutionResult readActualValue(
        const Device& device,
        const DeviceTemplate& deviceTemplate,
        DeviceValue& output
    ) const;

private:
    const DriverBindingResolver& resolver_;

    DriverExecutionResult validateDeviceAndTemplate(
        const Device& device,
        const DeviceTemplate& deviceTemplate
    ) const;
    DriverExecutionResult validateAction(
        const Device& device,
        const DeviceTemplate& deviceTemplate,
        const OutputDriverPort& port,
        const DriverActionRequest& request
    ) const;
};

#endif
