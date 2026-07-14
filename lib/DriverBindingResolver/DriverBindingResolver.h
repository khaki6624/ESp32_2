#ifndef DRIVER_BINDING_RESOLVER_H
#define DRIVER_BINDING_RESOLVER_H

#include <InputDriverPort.h>
#include <OutputDriverPort.h>

class DriverBindingResolver
{
public:
    virtual ~DriverBindingResolver() = default;
    virtual DriverExecutionResult resolveOutput(
        const DeviceBinding& binding,
        OutputDriverPort*& output
    ) const = 0;
    virtual DriverExecutionResult resolveInput(
        const DeviceBinding& binding,
        InputDriverPort*& output
    ) const = 0;
};

#endif
