#ifndef ENDPOINT_REGISTRY_H
#define ENDPOINT_REGISTRY_H

#include <stddef.h>

#include <HardwareEndpoint.h>

struct EndpointRegistryQuery
{
    NodeId nodeId;
    DriverType driverType;
    EndpointDirection direction;
    DeviceValueType valueType;
    EndpointAvailabilityFilter availability;
    EndpointBooleanFilter enabled;
    EndpointAssignmentFilter assignment;

    EndpointRegistryQuery();

    bool isValid() const;
    bool matches(const HardwareEndpoint& endpoint) const;
};

class EndpointRegistry
{
public:
    EndpointRegistry();

    void clear();

    EndpointRegistryResult add(const HardwareEndpoint& endpoint);
    EndpointRegistryResult update(const HardwareEndpoint& endpoint);
    EndpointRegistryResult remove(EndpointId id);

    HardwareEndpoint* findById(EndpointId id);
    const HardwareEndpoint* findById(EndpointId id) const;

    HardwareEndpoint* findByKey(const HardwareEndpointKey& key);
    const HardwareEndpoint* findByKey(const HardwareEndpointKey& key) const;

    HardwareEndpoint* findByAssignedDevice(DeviceId deviceId);
    const HardwareEndpoint* findByAssignedDevice(DeviceId deviceId) const;

    HardwareEndpoint* getAt(size_t index);
    const HardwareEndpoint* getAt(size_t index) const;

    bool contains(EndpointId id) const;
    bool containsKey(const HardwareEndpointKey& key) const;
    bool containsAssignedDevice(DeviceId deviceId) const;

    EndpointRegistryResult assignDevice(EndpointId endpointId, DeviceId deviceId);
    EndpointRegistryResult unassignDevice(EndpointId endpointId);

    size_t countByNode(NodeId nodeId) const;
    size_t countByDriver(DriverType driverType) const;
    size_t countAssigned() const;
    size_t countUnassigned() const;

    EndpointRegistryResult query(
        const EndpointRegistryQuery& query,
        EndpointId* outputIds,
        size_t outputCapacity,
        size_t& outputCount
    ) const;

    size_t size() const;
    size_t capacity() const;
    bool isFull() const;
    bool isEmpty() const;

private:
    HardwareEndpoint endpoints_[ENDPOINT_REGISTRY_CAPACITY];
    size_t count_;

    size_t findIndexById(EndpointId id) const;
    bool keyBelongsToAnotherEndpoint(
        const HardwareEndpointKey& key,
        EndpointId id
    ) const;
    bool deviceBelongsToAnotherEndpoint(DeviceId deviceId, EndpointId id) const;
    EndpointRegistryResult validateForRegistry(const HardwareEndpoint& endpoint) const;
};

#endif
