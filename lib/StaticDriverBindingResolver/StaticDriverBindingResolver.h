#ifndef STATIC_DRIVER_BINDING_RESOLVER_H
#define STATIC_DRIVER_BINDING_RESOLVER_H

#include <DriverBindingResolver.h>

struct OutputDriverBindingEntry
{
    DeviceBinding binding;
    OutputDriverPort* port;
    bool used;
};

struct InputDriverBindingEntry
{
    DeviceBinding binding;
    InputDriverPort* port;
    bool used;
};

class StaticDriverBindingResolver final : public DriverBindingResolver
{
public:
    explicit StaticDriverBindingResolver(NodeId localNodeId);
    void clear();

    DriverExecutionResult registerOutput(
        const DeviceBinding& binding,
        OutputDriverPort& port
    );
    DriverExecutionResult registerInput(
        const DeviceBinding& binding,
        InputDriverPort& port
    );
    DriverExecutionResult unregisterOutput(const DeviceBinding& binding);
    DriverExecutionResult unregisterInput(const DeviceBinding& binding);

    DriverExecutionResult resolveOutput(
        const DeviceBinding& binding,
        OutputDriverPort*& output
    ) const override;
    DriverExecutionResult resolveInput(
        const DeviceBinding& binding,
        InputDriverPort*& output
    ) const override;

    size_t outputCount() const;
    size_t inputCount() const;
    size_t outputCapacity() const;
    size_t inputCapacity() const;
    NodeId getLocalNodeId() const;

private:
    NodeId localNodeId_;
    OutputDriverBindingEntry outputs_[OUTPUT_DRIVER_BINDING_CAPACITY];
    InputDriverBindingEntry inputs_[INPUT_DRIVER_BINDING_CAPACITY];
    size_t outputCount_;
    size_t inputCount_;

    bool sameBinding(
        const DeviceBinding& left,
        const DeviceBinding& right
    ) const;
};

#endif
