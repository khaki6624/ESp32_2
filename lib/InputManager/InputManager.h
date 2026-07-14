#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <Device.h>
#include <DeviceTemplate.h>
#include <DriverBindingResolver.h>

class InputManager
{
public:
    explicit InputManager(const DriverBindingResolver& resolver);

    DriverExecutionResult read(
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
};

#endif
