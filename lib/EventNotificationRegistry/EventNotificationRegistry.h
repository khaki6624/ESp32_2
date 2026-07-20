#ifndef EVENT_NOTIFICATION_REGISTRY_H
#define EVENT_NOTIFICATION_REGISTRY_H
#include <EventNotificationMapping.h>
struct EventNotificationRegistryEntry{EventNotificationMapping mapping;char target[NOTIFICATION_RUNTIME_MAX_TARGET_LENGTH];EventNotificationRegistryEntry():mapping(),target{}{}};
class EventNotificationRegistry
{public:EventNotificationRegistry();RuntimeIntegrationResult registerMapping(const EventNotificationMapping&);const EventNotificationMapping* find(EventType)const;const EventNotificationMapping* at(size_t)const;size_t size()const;size_t capacity()const;bool isLocked()const;RuntimeIntegrationResult lock();private:EventNotificationRegistryEntry entries_[EVENT_NOTIFICATION_MAX_MAPPINGS];size_t size_;bool locked_;};
#endif
