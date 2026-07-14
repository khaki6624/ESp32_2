#include "StaticDriverBindingResolver.h"

StaticDriverBindingResolver::StaticDriverBindingResolver(NodeId localNodeId) :
    localNodeId_(localNodeId),
    outputs_{},
    inputs_{},
    outputCount_(0U),
    inputCount_(0U)
{
}

void StaticDriverBindingResolver::clear()
{
    for (size_t index = 0; index < OUTPUT_DRIVER_BINDING_CAPACITY; ++index)
        outputs_[index] = OutputDriverBindingEntry{};
    for (size_t index = 0; index < INPUT_DRIVER_BINDING_CAPACITY; ++index)
        inputs_[index] = InputDriverBindingEntry{};
    outputCount_ = 0U;
    inputCount_ = 0U;
}

DriverExecutionResult StaticDriverBindingResolver::registerOutput(
    const DeviceBinding& binding,
    OutputDriverPort& port
)
{
    if (!binding.isValid() || localNodeId_ == INVALID_NODE_ID)
        return DriverExecutionResult::INVALID_BINDING;
    if (binding.nodeId != localNodeId_)
        return DriverExecutionResult::REMOTE_BINDING_NOT_SUPPORTED;
    if (port.getDriverType() != binding.driverType)
        return DriverExecutionResult::INVALID_BINDING;

    size_t freeIndex = OUTPUT_DRIVER_BINDING_CAPACITY;
    for (size_t index = 0; index < OUTPUT_DRIVER_BINDING_CAPACITY; ++index)
    {
        if (!outputs_[index].used)
        {
            if (freeIndex == OUTPUT_DRIVER_BINDING_CAPACITY)
                freeIndex = index;
            continue;
        }
        if (sameBinding(outputs_[index].binding, binding))
            return DriverExecutionResult::BINDING_ALREADY_REGISTERED;
        if (outputs_[index].port == &port)
            return DriverExecutionResult::PORT_ALREADY_REGISTERED;
    }
    if (freeIndex == OUTPUT_DRIVER_BINDING_CAPACITY)
        return DriverExecutionResult::RESOLVER_FULL;

    outputs_[freeIndex].binding = binding;
    outputs_[freeIndex].port = &port;
    outputs_[freeIndex].used = true;
    ++outputCount_;
    return DriverExecutionResult::SUCCESS;
}

DriverExecutionResult StaticDriverBindingResolver::registerInput(
    const DeviceBinding& binding,
    InputDriverPort& port
)
{
    if (!binding.isValid() || localNodeId_ == INVALID_NODE_ID)
        return DriverExecutionResult::INVALID_BINDING;
    if (binding.nodeId != localNodeId_)
        return DriverExecutionResult::REMOTE_BINDING_NOT_SUPPORTED;
    if (port.getDriverType() != binding.driverType)
        return DriverExecutionResult::INVALID_BINDING;

    size_t freeIndex = INPUT_DRIVER_BINDING_CAPACITY;
    for (size_t index = 0; index < INPUT_DRIVER_BINDING_CAPACITY; ++index)
    {
        if (!inputs_[index].used)
        {
            if (freeIndex == INPUT_DRIVER_BINDING_CAPACITY)
                freeIndex = index;
            continue;
        }
        if (sameBinding(inputs_[index].binding, binding))
            return DriverExecutionResult::BINDING_ALREADY_REGISTERED;
        if (inputs_[index].port == &port)
            return DriverExecutionResult::PORT_ALREADY_REGISTERED;
    }
    if (freeIndex == INPUT_DRIVER_BINDING_CAPACITY)
        return DriverExecutionResult::RESOLVER_FULL;

    inputs_[freeIndex].binding = binding;
    inputs_[freeIndex].port = &port;
    inputs_[freeIndex].used = true;
    ++inputCount_;
    return DriverExecutionResult::SUCCESS;
}

DriverExecutionResult StaticDriverBindingResolver::unregisterOutput(
    const DeviceBinding& binding
)
{
    if (!binding.isValid())
        return DriverExecutionResult::INVALID_BINDING;
    if (binding.nodeId != localNodeId_)
        return DriverExecutionResult::REMOTE_BINDING_NOT_SUPPORTED;
    for (size_t index = 0; index < OUTPUT_DRIVER_BINDING_CAPACITY; ++index)
    {
        if (outputs_[index].used && sameBinding(outputs_[index].binding, binding))
        {
            // حذف Sparse است؛ هیچ Portی Destroy یا جابه‌جا نمی‌شود.
            outputs_[index] = OutputDriverBindingEntry{};
            --outputCount_;
            return DriverExecutionResult::SUCCESS;
        }
    }
    return DriverExecutionResult::BINDING_NOT_FOUND;
}

DriverExecutionResult StaticDriverBindingResolver::unregisterInput(
    const DeviceBinding& binding
)
{
    if (!binding.isValid())
        return DriverExecutionResult::INVALID_BINDING;
    if (binding.nodeId != localNodeId_)
        return DriverExecutionResult::REMOTE_BINDING_NOT_SUPPORTED;
    for (size_t index = 0; index < INPUT_DRIVER_BINDING_CAPACITY; ++index)
    {
        if (inputs_[index].used && sameBinding(inputs_[index].binding, binding))
        {
            // حذف Sparse است؛ Slot آزاد برای ثبت بعدی قابل استفاده می‌ماند.
            inputs_[index] = InputDriverBindingEntry{};
            --inputCount_;
            return DriverExecutionResult::SUCCESS;
        }
    }
    return DriverExecutionResult::BINDING_NOT_FOUND;
}

DriverExecutionResult StaticDriverBindingResolver::resolveOutput(
    const DeviceBinding& binding,
    OutputDriverPort*& output
) const
{
    if (!binding.isValid())
        return DriverExecutionResult::INVALID_BINDING;
    if (binding.nodeId != localNodeId_)
        return DriverExecutionResult::REMOTE_BINDING_NOT_SUPPORTED;
    for (size_t index = 0; index < OUTPUT_DRIVER_BINDING_CAPACITY; ++index)
    {
        const OutputDriverBindingEntry& entry = outputs_[index];
        if (!entry.used || !sameBinding(entry.binding, binding))
            continue;
        if (entry.port == nullptr || !entry.binding.isValid() ||
            entry.binding.driverType != binding.driverType ||
            entry.port->getDriverType() != binding.driverType)
            return DriverExecutionResult::INVALID_BINDING;
        output = entry.port;
        return DriverExecutionResult::SUCCESS;
    }
    return DriverExecutionResult::BINDING_NOT_FOUND;
}

DriverExecutionResult StaticDriverBindingResolver::resolveInput(
    const DeviceBinding& binding,
    InputDriverPort*& output
) const
{
    if (!binding.isValid())
        return DriverExecutionResult::INVALID_BINDING;
    if (binding.nodeId != localNodeId_)
        return DriverExecutionResult::REMOTE_BINDING_NOT_SUPPORTED;
    for (size_t index = 0; index < INPUT_DRIVER_BINDING_CAPACITY; ++index)
    {
        const InputDriverBindingEntry& entry = inputs_[index];
        if (!entry.used || !sameBinding(entry.binding, binding))
            continue;
        if (entry.port == nullptr || !entry.binding.isValid() ||
            entry.binding.driverType != binding.driverType ||
            entry.port->getDriverType() != binding.driverType)
            return DriverExecutionResult::INVALID_BINDING;
        output = entry.port;
        return DriverExecutionResult::SUCCESS;
    }
    return DriverExecutionResult::BINDING_NOT_FOUND;
}

size_t StaticDriverBindingResolver::outputCount() const { return outputCount_; }
size_t StaticDriverBindingResolver::inputCount() const { return inputCount_; }
size_t StaticDriverBindingResolver::outputCapacity() const
{
    return OUTPUT_DRIVER_BINDING_CAPACITY;
}
size_t StaticDriverBindingResolver::inputCapacity() const
{
    return INPUT_DRIVER_BINDING_CAPACITY;
}
NodeId StaticDriverBindingResolver::getLocalNodeId() const { return localNodeId_; }

bool StaticDriverBindingResolver::sameBinding(
    const DeviceBinding& left,
    const DeviceBinding& right
) const
{
    return left.nodeId == right.nodeId && left.driverType == right.driverType &&
           left.driverInstance == right.driverInstance && left.channel == right.channel;
}
