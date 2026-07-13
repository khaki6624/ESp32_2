#include "EndpointRegistry.h"

namespace
{
    constexpr size_t INVALID_ENDPOINT_INDEX = ENDPOINT_REGISTRY_CAPACITY;

    bool isValidBooleanFilter(EndpointBooleanFilter filter)
    {
        return static_cast<uint8_t>(filter) <=
               static_cast<uint8_t>(EndpointBooleanFilter::FALSE_ONLY);
    }

    bool matchesBoolean(EndpointBooleanFilter filter, bool value)
    {
        switch (filter)
        {
            case EndpointBooleanFilter::ANY: return true;
            case EndpointBooleanFilter::TRUE_ONLY: return value;
            case EndpointBooleanFilter::FALSE_ONLY: return !value;
            default: return false;
        }
    }

    bool matchesAvailability(
        EndpointAvailabilityFilter filter,
        EndpointAvailability availability
    )
    {
        if (filter == EndpointAvailabilityFilter::ANY)
            return true;
        return static_cast<uint8_t>(filter) == static_cast<uint8_t>(availability) + 1U;
    }

    bool matchesAssignment(
        EndpointAssignmentFilter filter,
        bool assigned
    )
    {
        switch (filter)
        {
            case EndpointAssignmentFilter::ANY: return true;
            case EndpointAssignmentFilter::ASSIGNED_ONLY: return assigned;
            case EndpointAssignmentFilter::UNASSIGNED_ONLY: return !assigned;
            default: return false;
        }
    }
}

EndpointRegistryQuery::EndpointRegistryQuery() :
    nodeId(INVALID_NODE_ID),
    driverType(DriverType::NONE),
    direction(EndpointDirection::NONE),
    valueType(DeviceValueType::NONE),
    availability(EndpointAvailabilityFilter::ANY),
    enabled(EndpointBooleanFilter::ANY),
    assignment(EndpointAssignmentFilter::ANY)
{
}

bool EndpointRegistryQuery::isValid() const
{
    return isValidDriverType(driverType) && isValidEndpointDirection(direction) &&
           isValidDeviceValueType(valueType) &&
           static_cast<uint8_t>(availability) <=
               static_cast<uint8_t>(EndpointAvailabilityFilter::ERROR_ONLY) &&
           isValidBooleanFilter(enabled) &&
           static_cast<uint8_t>(assignment) <=
               static_cast<uint8_t>(EndpointAssignmentFilter::UNASSIGNED_ONLY);
}

bool EndpointRegistryQuery::matches(const HardwareEndpoint& endpoint) const
{
    if (!isValid())
        return false;
    if (nodeId != INVALID_NODE_ID && endpoint.key.nodeId != nodeId)
        return false;
    if (driverType != DriverType::NONE && endpoint.key.driverType != driverType)
        return false;
    if (direction != EndpointDirection::NONE && endpoint.direction != direction)
        return false;
    if (valueType != DeviceValueType::NONE && endpoint.valueType != valueType)
        return false;
    return matchesAvailability(availability, endpoint.availability) &&
           matchesBoolean(enabled, endpoint.enabled) &&
           matchesAssignment(assignment, endpoint.isAssigned());
}

EndpointRegistry::EndpointRegistry() :
    endpoints_{},
    count_(0)
{
}

void EndpointRegistry::clear()
{
    for (size_t index = 0; index < ENDPOINT_REGISTRY_CAPACITY; ++index)
        endpoints_[index] = HardwareEndpoint{};
    count_ = 0;
}

EndpointRegistryResult EndpointRegistry::add(const HardwareEndpoint& endpoint)
{
    const EndpointRegistryResult validationResult = validateForRegistry(endpoint);
    if (validationResult != EndpointRegistryResult::SUCCESS)
        return validationResult;
    if (isFull())
        return EndpointRegistryResult::CAPACITY_FULL;
    if (contains(endpoint.id))
        return EndpointRegistryResult::DUPLICATE_ID;
    if (containsKey(endpoint.key))
        return EndpointRegistryResult::DUPLICATE_KEY;
    if (endpoint.isAssigned() && containsAssignedDevice(endpoint.assignedDeviceId))
        return EndpointRegistryResult::DUPLICATE_DEVICE_ASSIGNMENT;

    endpoints_[count_] = endpoint;
    ++count_;
    return EndpointRegistryResult::SUCCESS;
}

EndpointRegistryResult EndpointRegistry::update(const HardwareEndpoint& endpoint)
{
    const EndpointRegistryResult validationResult = validateForRegistry(endpoint);
    if (validationResult != EndpointRegistryResult::SUCCESS)
        return validationResult;

    const size_t index = findIndexById(endpoint.id);
    if (index == INVALID_ENDPOINT_INDEX)
        return EndpointRegistryResult::NOT_FOUND;
    if (keyBelongsToAnotherEndpoint(endpoint.key, endpoint.id))
        return EndpointRegistryResult::DUPLICATE_KEY;
    if (endpoint.isAssigned() &&
        deviceBelongsToAnotherEndpoint(endpoint.assignedDeviceId, endpoint.id))
    {
        return EndpointRegistryResult::DUPLICATE_DEVICE_ASSIGNMENT;
    }

    endpoints_[index] = endpoint;
    return EndpointRegistryResult::SUCCESS;
}

EndpointRegistryResult EndpointRegistry::remove(EndpointId id)
{
    if (id == INVALID_ENDPOINT_ID)
        return EndpointRegistryResult::INVALID_ID;

    const size_t index = findIndexById(id);
    if (index == INVALID_ENDPOINT_INDEX)
        return EndpointRegistryResult::NOT_FOUND;

    // Registry فقط Inventory است؛ Endpoint تخصیص‌یافته نیز قابل حذف است.
    for (size_t current = index + 1U; current < count_; ++current)
        endpoints_[current - 1U] = endpoints_[current];
    --count_;
    endpoints_[count_] = HardwareEndpoint{};
    return EndpointRegistryResult::SUCCESS;
}

HardwareEndpoint* EndpointRegistry::findById(EndpointId id)
{
    const size_t index = findIndexById(id);
    return index == INVALID_ENDPOINT_INDEX ? nullptr : &endpoints_[index];
}

const HardwareEndpoint* EndpointRegistry::findById(EndpointId id) const
{
    const size_t index = findIndexById(id);
    return index == INVALID_ENDPOINT_INDEX ? nullptr : &endpoints_[index];
}

HardwareEndpoint* EndpointRegistry::findByKey(const HardwareEndpointKey& key)
{
    if (!key.isValid())
        return nullptr;
    for (size_t index = 0; index < count_; ++index)
    {
        if (endpoints_[index].key == key)
            return &endpoints_[index];
    }
    return nullptr;
}

const HardwareEndpoint* EndpointRegistry::findByKey(const HardwareEndpointKey& key) const
{
    if (!key.isValid())
        return nullptr;
    for (size_t index = 0; index < count_; ++index)
    {
        if (endpoints_[index].key == key)
            return &endpoints_[index];
    }
    return nullptr;
}

HardwareEndpoint* EndpointRegistry::findByAssignedDevice(DeviceId deviceId)
{
    if (deviceId == INVALID_DEVICE_ID)
        return nullptr;
    for (size_t index = 0; index < count_; ++index)
    {
        if (endpoints_[index].assignedDeviceId == deviceId)
            return &endpoints_[index];
    }
    return nullptr;
}

const HardwareEndpoint* EndpointRegistry::findByAssignedDevice(DeviceId deviceId) const
{
    if (deviceId == INVALID_DEVICE_ID)
        return nullptr;
    for (size_t index = 0; index < count_; ++index)
    {
        if (endpoints_[index].assignedDeviceId == deviceId)
            return &endpoints_[index];
    }
    return nullptr;
}

HardwareEndpoint* EndpointRegistry::getAt(size_t index)
{
    return index < count_ ? &endpoints_[index] : nullptr;
}

const HardwareEndpoint* EndpointRegistry::getAt(size_t index) const
{
    return index < count_ ? &endpoints_[index] : nullptr;
}

bool EndpointRegistry::contains(EndpointId id) const
{
    return findById(id) != nullptr;
}

bool EndpointRegistry::containsKey(const HardwareEndpointKey& key) const
{
    return findByKey(key) != nullptr;
}

bool EndpointRegistry::containsAssignedDevice(DeviceId deviceId) const
{
    return findByAssignedDevice(deviceId) != nullptr;
}

EndpointRegistryResult EndpointRegistry::assignDevice(
    EndpointId endpointId,
    DeviceId deviceId
)
{
    if (endpointId == INVALID_ENDPOINT_ID)
        return EndpointRegistryResult::INVALID_ID;
    if (deviceId == INVALID_DEVICE_ID)
        return EndpointRegistryResult::INVALID_DEVICE_ID;

    HardwareEndpoint* endpoint = findById(endpointId);
    if (endpoint == nullptr)
        return EndpointRegistryResult::NOT_FOUND;
    if (!endpoint->enabled)
        return EndpointRegistryResult::ENDPOINT_DISABLED;
    if (endpoint->availability != EndpointAvailability::AVAILABLE)
        return EndpointRegistryResult::ENDPOINT_UNAVAILABLE;
    if (endpoint->assignedDeviceId == deviceId)
        return EndpointRegistryResult::SUCCESS;
    if (endpoint->isAssigned())
        return EndpointRegistryResult::ALREADY_ASSIGNED;
    if (containsAssignedDevice(deviceId))
        return EndpointRegistryResult::DUPLICATE_DEVICE_ASSIGNMENT;

    endpoint->assignedDeviceId = deviceId;
    return EndpointRegistryResult::SUCCESS;
}

EndpointRegistryResult EndpointRegistry::unassignDevice(EndpointId endpointId)
{
    if (endpointId == INVALID_ENDPOINT_ID)
        return EndpointRegistryResult::INVALID_ID;
    HardwareEndpoint* endpoint = findById(endpointId);
    if (endpoint == nullptr)
        return EndpointRegistryResult::NOT_FOUND;
    if (!endpoint->isAssigned())
        return EndpointRegistryResult::NOT_ASSIGNED;

    endpoint->assignedDeviceId = INVALID_DEVICE_ID;
    return EndpointRegistryResult::SUCCESS;
}

size_t EndpointRegistry::countByNode(NodeId nodeId) const
{
    if (nodeId == INVALID_NODE_ID)
        return 0;
    size_t result = 0;
    for (size_t index = 0; index < count_; ++index)
    {
        if (endpoints_[index].key.nodeId == nodeId)
            ++result;
    }
    return result;
}

size_t EndpointRegistry::countByDriver(DriverType driverType) const
{
    if (driverType == DriverType::NONE || !isValidDriverType(driverType))
        return 0;
    size_t result = 0;
    for (size_t index = 0; index < count_; ++index)
    {
        if (endpoints_[index].key.driverType == driverType)
            ++result;
    }
    return result;
}

size_t EndpointRegistry::countAssigned() const
{
    size_t result = 0;
    for (size_t index = 0; index < count_; ++index)
        result += endpoints_[index].isAssigned() ? 1U : 0U;
    return result;
}

size_t EndpointRegistry::countUnassigned() const
{
    return count_ - countAssigned();
}

EndpointRegistryResult EndpointRegistry::query(
    const EndpointRegistryQuery& queryValue,
    EndpointId* outputIds,
    size_t outputCapacity,
    size_t& outputCount
) const
{
    outputCount = 0;
    if (!queryValue.isValid())
        return EndpointRegistryResult::INVALID_ENDPOINT;
    if (outputIds == nullptr && outputCapacity > 0U)
        return EndpointRegistryResult::OUTPUT_BUFFER_INVALID;

    size_t requiredCount = 0;
    for (size_t index = 0; index < count_; ++index)
    {
        if (queryValue.matches(endpoints_[index]))
            ++requiredCount;
    }
    if (requiredCount == 0U)
        return EndpointRegistryResult::NO_MATCHES;
    if (requiredCount > outputCapacity)
    {
        outputCount = requiredCount;
        return EndpointRegistryResult::OUTPUT_BUFFER_TOO_SMALL;
    }

    for (size_t index = 0; index < count_; ++index)
    {
        if (queryValue.matches(endpoints_[index]))
            outputIds[outputCount++] = endpoints_[index].id;
    }
    return EndpointRegistryResult::SUCCESS;
}

size_t EndpointRegistry::size() const
{
    return count_;
}

size_t EndpointRegistry::capacity() const
{
    return ENDPOINT_REGISTRY_CAPACITY;
}

bool EndpointRegistry::isFull() const
{
    return count_ >= ENDPOINT_REGISTRY_CAPACITY;
}

bool EndpointRegistry::isEmpty() const
{
    return count_ == 0U;
}

size_t EndpointRegistry::findIndexById(EndpointId id) const
{
    if (id == INVALID_ENDPOINT_ID)
        return INVALID_ENDPOINT_INDEX;
    for (size_t index = 0; index < count_; ++index)
    {
        if (endpoints_[index].id == id)
            return index;
    }
    return INVALID_ENDPOINT_INDEX;
}

bool EndpointRegistry::keyBelongsToAnotherEndpoint(
    const HardwareEndpointKey& key,
    EndpointId id
) const
{
    if (!key.isValid())
        return true;
    for (size_t index = 0; index < count_; ++index)
    {
        if (endpoints_[index].id != id && endpoints_[index].key == key)
            return true;
    }
    return false;
}

bool EndpointRegistry::deviceBelongsToAnotherEndpoint(
    DeviceId deviceId,
    EndpointId id
) const
{
    if (deviceId == INVALID_DEVICE_ID)
        return false;
    for (size_t index = 0; index < count_; ++index)
    {
        if (endpoints_[index].id != id && endpoints_[index].assignedDeviceId == deviceId)
            return true;
    }
    return false;
}

EndpointRegistryResult EndpointRegistry::validateForRegistry(
    const HardwareEndpoint& endpoint
) const
{
    if (endpoint.id == INVALID_ENDPOINT_ID)
        return EndpointRegistryResult::INVALID_ID;
    if (!endpoint.key.isValid())
        return EndpointRegistryResult::INVALID_KEY;
    if (endpoint.direction == EndpointDirection::NONE ||
        !isValidEndpointDirection(endpoint.direction))
    {
        return EndpointRegistryResult::INVALID_DIRECTION;
    }
    if (endpoint.valueType == DeviceValueType::NONE ||
        !isValidDeviceValueType(endpoint.valueType))
    {
        return EndpointRegistryResult::INVALID_VALUE_TYPE;
    }
    if (!isValidActionMask(endpoint.supportedActions))
        return EndpointRegistryResult::INVALID_ACTION_MASK;
    if (!isValidEndpointAvailability(endpoint.availability) || !endpoint.isValid())
        return EndpointRegistryResult::INVALID_ENDPOINT;
    return EndpointRegistryResult::SUCCESS;
}
