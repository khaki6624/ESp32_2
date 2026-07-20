#ifndef NOTIFICATION_RUNTIME_H
#define NOTIFICATION_RUNTIME_H
#include <NotificationRegistry.h>
#include <NotificationRuntimeState.h>

struct NotificationRuntimeSlot
{
 const NotificationChannelDescriptor* descriptor;
 NotificationSink* sink;
 NotificationRuntimeState state;
 NotificationRuntimeSlot():descriptor(nullptr),sink(nullptr),state(){}
};

class NotificationRuntime
{
public:
 explicit NotificationRuntime(NotificationRegistry& registry);
 NotificationRuntimeResult begin(); bool isInitialized()const;
 NotificationRuntimeResult send(const NotificationMessage& message,NotificationTimestamp now);
 NotificationRuntimeResult update(NotificationTimestamp now);
 NotificationRuntimeResult cancel(NotificationChannelId channelId);
 const NotificationRuntimeState* state(NotificationChannelId channelId)const;
 bool isBusy(NotificationChannelId channelId)const;
private:
 NotificationRegistry& registry_; NotificationRuntimeSlot slots_[NOTIFICATION_RUNTIME_MAX_CHANNELS];
 size_t slotCount_; bool initialized_;
 size_t findSlot(NotificationChannelId channelId)const;
 NotificationRuntimeResult applySendResult(NotificationRuntimeSlot& slot,NotificationId id,NotificationSinkResult result);
 NotificationRuntimeResult applyActiveResult(NotificationRuntimeSlot& slot,NotificationSinkResult result,bool cancelling);
};
#endif
