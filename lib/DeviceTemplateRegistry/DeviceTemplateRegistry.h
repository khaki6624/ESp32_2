#ifndef DEVICE_TEMPLATE_REGISTRY_H
#define DEVICE_TEMPLATE_REGISTRY_H

#include <stddef.h>

#include <DeviceTemplate.h>

constexpr size_t DEVICE_TEMPLATE_REGISTRY_CAPACITY = 24;

class DeviceTemplateRegistry
{
public:
    DeviceTemplateRegistry();

    void clear();

    bool add(const DeviceTemplate& deviceTemplate);
    bool update(const DeviceTemplate& deviceTemplate);
    bool remove(DeviceTemplateId id);

    DeviceTemplate* findById(DeviceTemplateId id);
    const DeviceTemplate* findById(DeviceTemplateId id) const;

    DeviceTemplate* findByName(const char* name);
    const DeviceTemplate* findByName(const char* name) const;

    DeviceTemplate* getAt(size_t index);
    const DeviceTemplate* getAt(size_t index) const;

    bool contains(DeviceTemplateId id) const;
    bool containsName(const char* name) const;

    bool supportsAction(DeviceTemplateId id, DeviceAction action) const;
    bool supportsDriver(DeviceTemplateId id, DriverType driverType) const;

    size_t size() const;
    size_t capacity() const;
    bool isFull() const;
    bool isEmpty() const;

private:
    DeviceTemplate templates_[DEVICE_TEMPLATE_REGISTRY_CAPACITY];
    size_t count_;

    size_t findIndexById(DeviceTemplateId id) const;
    bool nameBelongsToAnotherTemplate(const char* name, DeviceTemplateId id) const;
};

#endif
