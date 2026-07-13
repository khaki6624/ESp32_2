#ifndef DEVICE_REGISTRY_H
#define DEVICE_REGISTRY_H

#include <stddef.h>

#include <DeviceRegistryCommon.h>
#include <DeviceRegistryQuery.h>
#include <DeviceTemplateRegistry.h>

class DeviceRegistry
{
public:
    explicit DeviceRegistry(const DeviceTemplateRegistry& templateRegistry);

    void clear();

    DeviceRegistryResult add(const Device& device);
    DeviceRegistryResult update(const Device& device);
    DeviceRegistryResult remove(DeviceId id);

    Device* findById(DeviceId id);
    const Device* findById(DeviceId id) const;

    Device* findByName(const char* name);
    const Device* findByName(const char* name) const;

    Device* getAt(size_t index);
    const Device* getAt(size_t index) const;

    bool contains(DeviceId id) const;
    bool containsName(const char* name) const;
    bool containsBinding(const DeviceBinding& binding) const;

    size_t countByNode(NodeId nodeId) const;
    size_t countByLocation(LocationId locationId) const;
    size_t countByTemplate(DeviceTemplateId templateId) const;

    DeviceRegistryResult query(
        const DeviceRegistryQuery& query,
        DeviceId* outputIds,
        size_t outputCapacity,
        size_t& outputCount
    ) const;

    size_t size() const;
    size_t capacity() const;
    bool isFull() const;
    bool isEmpty() const;

private:
    const DeviceTemplateRegistry& templateRegistry_;
    Device devices_[DEVICE_REGISTRY_CAPACITY];
    size_t count_;

    size_t findIndexById(DeviceId id) const;
    bool nameBelongsToAnotherDevice(const char* name, DeviceId id) const;
    bool bindingBelongsToAnotherDevice(
        const DeviceBinding& binding,
        DeviceId id
    ) const;
    DeviceRegistryResult validateForRegistry(const Device& device) const;
};

#endif
