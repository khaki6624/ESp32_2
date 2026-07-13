#ifndef NODE_PROFILE_H
#define NODE_PROFILE_H

#include <string.h>

#include <NodeCommon.h>

// حقیقت مستقل تنظیمات یک Node؛ فاقد هرگونه وضعیت Runtime است.
struct NodeProfile
{
    NodeId id;
    char name[NODE_NAME_MAX_LENGTH];
    NodeRole role;
    NodeLinkType linkType;
    bool enabled;
    bool configured;

    NodeProfile() :
        id(INVALID_NODE_ID),
        name{},
        role(NodeRole::NONE),
        linkType(NodeLinkType::NONE),
        enabled(false),
        configured(false)
    {
    }

    bool isValid() const
    {
        return id != INVALID_NODE_ID && name[0] != '\0' &&
               memchr(name, '\0', NODE_NAME_MAX_LENGTH) != nullptr &&
               role != NodeRole::NONE && isValidNodeRole(role) &&
               linkType != NodeLinkType::NONE && isValidNodeLinkType(linkType);
    }

    bool isOperationallyConfigured() const
    {
        return isValid() && enabled && configured;
    }

    bool setName(const char* value)
    {
        if (value == nullptr || value[0] == '\0')
            return false;

        size_t length = 0;
        while (length < NODE_NAME_MAX_LENGTH && value[length] != '\0')
            ++length;
        if (length >= NODE_NAME_MAX_LENGTH)
            return false;

        memset(name, 0, sizeof(name));
        memcpy(name, value, length);
        return true;
    }
};

#endif
