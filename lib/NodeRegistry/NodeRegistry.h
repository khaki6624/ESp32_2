#ifndef NODE_REGISTRY_H
#define NODE_REGISTRY_H

#include <stddef.h>

#include <NodeHealth.h>
#include <NodeProfile.h>

struct NodeRegistryQuery
{
    NodeRole role;
    NodeLinkType linkType;
    NodeHealthFilter health;
    NodeBooleanFilter enabled;
    NodeBooleanFilter configured;

    NodeRegistryQuery();
    bool isValid() const;
    bool matches(const NodeProfile& profile, const NodeHealth& healthValue) const;
};

class NodeRegistry
{
public:
    NodeRegistry();

    void clear();
    NodeRegistryResult add(const NodeProfile& profile);
    NodeRegistryResult updateProfile(const NodeProfile& profile);
    NodeRegistryResult remove(NodeId id);
    NodeRegistryResult updateHealth(const NodeHealth& healthValue);
    NodeRegistryResult resetHealth(NodeId id);

    const NodeProfile* findProfileById(NodeId id) const;
    const NodeProfile* findProfileByName(const char* name) const;
    const NodeHealth* findHealthById(NodeId id) const;
    const NodeProfile* getProfileAt(size_t index) const;
    const NodeHealth* getHealthAt(size_t index) const;

    bool contains(NodeId id) const;
    bool containsName(const char* name) const;
    size_t countByRole(NodeRole role) const;
    size_t countByLinkType(NodeLinkType type) const;
    size_t countByHealth(NodeHealthState state) const;
    NodeRegistryResult query(const NodeRegistryQuery& query, NodeId* outputIds,
                             size_t outputCapacity, size_t& outputCount) const;

    size_t size() const;
    size_t capacity() const;
    bool isFull() const;
    bool isEmpty() const;

private:
    NodeProfile profiles_[NODE_REGISTRY_CAPACITY];
    NodeHealth health_[NODE_REGISTRY_CAPACITY];
    size_t count_;

    size_t findIndexById(NodeId id) const;
    bool nameBelongsToAnotherNode(const char* name, NodeId id) const;
    NodeRegistryResult validateProfile(const NodeProfile& profile) const;
    bool hasConsistentSlot(size_t index) const;
};

#endif
