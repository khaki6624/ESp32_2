#ifndef NOTIFICATION_REGISTRY_H
#define NOTIFICATION_REGISTRY_H
#include <NotificationSink.h>
class NotificationRegistry
{
public:
 NotificationRegistry();
 NotificationRuntimeResult registerChannel(const NotificationChannelDescriptor& descriptor,NotificationSink& sink);
 const NotificationChannelDescriptor* findDescriptor(NotificationChannelId channelId)const;
 NotificationSink* findSink(NotificationChannelId channelId)const;
 const NotificationChannelDescriptor* descriptorAt(size_t index)const;
 NotificationSink* sinkAt(size_t index)const;
 size_t size()const; size_t capacity()const; bool isLocked()const;
 NotificationRuntimeResult lock();
private:
 NotificationChannelDescriptor descriptors_[NOTIFICATION_RUNTIME_MAX_CHANNELS];
 NotificationSink* sinks_[NOTIFICATION_RUNTIME_MAX_CHANNELS];
 size_t size_; bool locked_;
};
#endif
