#include "NodeRegistry.h"

#include <string.h>

namespace
{
constexpr size_t INVALID_NODE_INDEX = NODE_REGISTRY_CAPACITY;

bool hasValidName(const char* name)
{
    if (name == nullptr || name[0] == '\0')
        return false;
    size_t length = 1;
    while (length < NODE_NAME_MAX_LENGTH && name[length] != '\0')
        ++length;
    return length < NODE_NAME_MAX_LENGTH;
}

bool matchesBoolean(bool value, NodeBooleanFilter filter)
{
    return filter == NodeBooleanFilter::ANY ||
           (filter == NodeBooleanFilter::TRUE_ONLY && value) ||
           (filter == NodeBooleanFilter::FALSE_ONLY && !value);
}
}

NodeRegistryQuery::NodeRegistryQuery() :
    role(NodeRole::NONE), linkType(NodeLinkType::NONE),
    health(NodeHealthFilter::ANY), enabled(NodeBooleanFilter::ANY),
    configured(NodeBooleanFilter::ANY)
{
}

bool NodeRegistryQuery::isValid() const
{
    return isValidNodeRole(role) && isValidNodeLinkType(linkType) &&
           isValidNodeHealthFilter(health) && isValidNodeBooleanFilter(enabled) &&
           isValidNodeBooleanFilter(configured);
}

bool NodeRegistryQuery::matches(const NodeProfile& profile,
                                const NodeHealth& healthValue) const
{
    if (!isValid() || !profile.isValid() || !healthValue.isValid() ||
        profile.id != healthValue.nodeId)
        return false;
    if (role != NodeRole::NONE && profile.role != role)
        return false;
    if (linkType != NodeLinkType::NONE && profile.linkType != linkType)
        return false;
    if (health != NodeHealthFilter::ANY &&
        static_cast<uint8_t>(health) != static_cast<uint8_t>(healthValue.state) + 1U)
        return false;
    return matchesBoolean(profile.enabled, enabled) &&
           matchesBoolean(profile.configured, configured);
}

NodeRegistry::NodeRegistry() : profiles_{}, health_{}, count_(0) {}

void NodeRegistry::clear()
{
    for (size_t i = 0; i < NODE_REGISTRY_CAPACITY; ++i)
    {
        profiles_[i] = NodeProfile{};
        health_[i] = NodeHealth{};
    }
    count_ = 0;
}

NodeRegistryResult NodeRegistry::validateProfile(const NodeProfile& profile) const
{
    if (profile.id == INVALID_NODE_ID) return NodeRegistryResult::INVALID_ID;
    if (!hasValidName(profile.name)) return NodeRegistryResult::INVALID_NAME;
    if (profile.role == NodeRole::NONE || !isValidNodeRole(profile.role))
        return NodeRegistryResult::INVALID_ROLE;
    if (profile.linkType == NodeLinkType::NONE || !isValidNodeLinkType(profile.linkType))
        return NodeRegistryResult::INVALID_LINK_TYPE;
    if (!profile.isValid()) return NodeRegistryResult::INVALID_NODE;
    return NodeRegistryResult::SUCCESS;
}

NodeRegistryResult NodeRegistry::add(const NodeProfile& profile)
{
    const NodeRegistryResult result = validateProfile(profile);
    if (result != NodeRegistryResult::SUCCESS) return result;
    if (isFull()) return NodeRegistryResult::CAPACITY_FULL;
    if (contains(profile.id)) return NodeRegistryResult::DUPLICATE_ID;
    if (containsName(profile.name)) return NodeRegistryResult::DUPLICATE_NAME;

    profiles_[count_] = profile;
    health_[count_] = NodeHealth{};
    health_[count_].nodeId = profile.id;
    ++count_;
    return NodeRegistryResult::SUCCESS;
}

NodeRegistryResult NodeRegistry::updateProfile(const NodeProfile& profile)
{
    const NodeRegistryResult result = validateProfile(profile);
    if (result != NodeRegistryResult::SUCCESS) return result;
    const size_t index = findIndexById(profile.id);
    if (index == INVALID_NODE_INDEX) return NodeRegistryResult::NOT_FOUND;
    if (nameBelongsToAnotherNode(profile.name, profile.id))
        return NodeRegistryResult::DUPLICATE_NAME;
    profiles_[index] = profile;
    return NodeRegistryResult::SUCCESS;
}

NodeRegistryResult NodeRegistry::updateHealth(const NodeHealth& healthValue)
{
    if (healthValue.nodeId == INVALID_NODE_ID) return NodeRegistryResult::INVALID_ID;
    if (!healthValue.isValid()) return NodeRegistryResult::INVALID_HEALTH;
    const size_t index = findIndexById(healthValue.nodeId);
    if (index == INVALID_NODE_INDEX) return NodeRegistryResult::NOT_FOUND;
    health_[index] = healthValue;
    return NodeRegistryResult::SUCCESS;
}

NodeRegistryResult NodeRegistry::resetHealth(NodeId id)
{
    if (id == INVALID_NODE_ID) return NodeRegistryResult::INVALID_ID;
    const size_t index = findIndexById(id);
    if (index == INVALID_NODE_INDEX) return NodeRegistryResult::NOT_FOUND;
    NodeHealth replacement;
    replacement.nodeId = id;
    health_[index] = replacement;
    return NodeRegistryResult::SUCCESS;
}

NodeRegistryResult NodeRegistry::remove(NodeId id)
{
    if (id == INVALID_NODE_ID) return NodeRegistryResult::INVALID_ID;
    const size_t index = findIndexById(id);
    if (index == INVALID_NODE_INDEX) return NodeRegistryResult::NOT_FOUND;
    for (size_t i = index + 1U; i < count_; ++i)
    {
        profiles_[i - 1U] = profiles_[i];
        health_[i - 1U] = health_[i];
    }
    --count_;
    profiles_[count_] = NodeProfile{};
    health_[count_] = NodeHealth{};
    return NodeRegistryResult::SUCCESS;
}

size_t NodeRegistry::findIndexById(NodeId id) const
{
    if (id == INVALID_NODE_ID) return INVALID_NODE_INDEX;
    for (size_t i = 0; i < count_; ++i)
        if (hasConsistentSlot(i) && profiles_[i].id == id) return i;
    return INVALID_NODE_INDEX;
}

const NodeProfile* NodeRegistry::findProfileById(NodeId id) const
{
    const size_t index = findIndexById(id);
    return index == INVALID_NODE_INDEX ? nullptr : &profiles_[index];
}

const NodeProfile* NodeRegistry::findProfileByName(const char* name) const
{
    if (!hasValidName(name)) return nullptr;
    for (size_t i = 0; i < count_; ++i)
        if (hasConsistentSlot(i) && strcmp(profiles_[i].name, name) == 0)
            return &profiles_[i];
    return nullptr;
}

const NodeHealth* NodeRegistry::findHealthById(NodeId id) const
{
    const size_t index = findIndexById(id);
    return index == INVALID_NODE_INDEX ? nullptr : &health_[index];
}

const NodeProfile* NodeRegistry::getProfileAt(size_t index) const
{
    return hasConsistentSlot(index) ? &profiles_[index] : nullptr;
}

const NodeHealth* NodeRegistry::getHealthAt(size_t index) const
{
    return hasConsistentSlot(index) ? &health_[index] : nullptr;
}

bool NodeRegistry::contains(NodeId id) const { return findIndexById(id) != INVALID_NODE_INDEX; }
bool NodeRegistry::containsName(const char* name) const { return findProfileByName(name) != nullptr; }

bool NodeRegistry::nameBelongsToAnotherNode(const char* name, NodeId id) const
{
    const NodeProfile* profile = findProfileByName(name);
    return profile != nullptr && profile->id != id;
}

size_t NodeRegistry::countByRole(NodeRole role) const
{
    if (role == NodeRole::NONE || !isValidNodeRole(role)) return 0;
    size_t result = 0;
    for (size_t i = 0; i < count_; ++i)
        if (hasConsistentSlot(i) && profiles_[i].role == role) ++result;
    return result;
}

size_t NodeRegistry::countByLinkType(NodeLinkType type) const
{
    if (type == NodeLinkType::NONE || !isValidNodeLinkType(type)) return 0;
    size_t result = 0;
    for (size_t i = 0; i < count_; ++i)
        if (hasConsistentSlot(i) && profiles_[i].linkType == type) ++result;
    return result;
}

size_t NodeRegistry::countByHealth(NodeHealthState state) const
{
    if (!isValidNodeHealthState(state)) return 0;
    size_t result = 0;
    for (size_t i = 0; i < count_; ++i)
        if (hasConsistentSlot(i) && health_[i].state == state) ++result;
    return result;
}

NodeRegistryResult NodeRegistry::query(const NodeRegistryQuery& queryValue,
                                       NodeId* outputIds, size_t outputCapacity,
                                       size_t& outputCount) const
{
    outputCount = 0;
    if (!queryValue.isValid()) return NodeRegistryResult::INVALID_NODE;
    if (outputIds == nullptr && outputCapacity > 0)
        return NodeRegistryResult::OUTPUT_BUFFER_INVALID;

    size_t required = 0;
    for (size_t i = 0; i < count_; ++i)
        if (hasConsistentSlot(i) && queryValue.matches(profiles_[i], health_[i])) ++required;
    outputCount = required;
    if (required == 0) return NodeRegistryResult::NO_MATCHES;
    if (outputIds == nullptr && outputCapacity == 0) return NodeRegistryResult::SUCCESS;
    if (outputCapacity < required) return NodeRegistryResult::OUTPUT_BUFFER_TOO_SMALL;

    size_t written = 0;
    for (size_t i = 0; i < count_; ++i)
        if (hasConsistentSlot(i) && queryValue.matches(profiles_[i], health_[i]))
            outputIds[written++] = profiles_[i].id;
    return NodeRegistryResult::SUCCESS;
}

bool NodeRegistry::hasConsistentSlot(size_t index) const
{
    return index < count_ && profiles_[index].isValid() && health_[index].isValid() &&
           profiles_[index].id == health_[index].nodeId;
}

size_t NodeRegistry::size() const { return count_; }
size_t NodeRegistry::capacity() const { return NODE_REGISTRY_CAPACITY; }
bool NodeRegistry::isFull() const { return count_ >= NODE_REGISTRY_CAPACITY; }
bool NodeRegistry::isEmpty() const { return count_ == 0; }
