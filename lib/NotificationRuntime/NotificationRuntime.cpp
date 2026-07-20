#include "NotificationRuntime.h"
namespace
{
constexpr size_t INVALID_SLOT=NOTIFICATION_RUNTIME_MAX_CHANNELS;
uint8_t updatePriority(NotificationRuntimeResult result)
{
 if(result==NotificationRuntimeResult::INTERNAL_ERROR)return 10U;
 if(result==NotificationRuntimeResult::SINK_FAILED)return 9U;
 if(result==NotificationRuntimeResult::SINK_REJECTED)return 8U;
 if(result==NotificationRuntimeResult::CANCELLED)return 7U;
 if(result==NotificationRuntimeResult::EXPIRED)return 6U;
 if(result==NotificationRuntimeResult::SUCCESS)return 5U;
 if(result==NotificationRuntimeResult::IN_PROGRESS)return 4U;
 if(result==NotificationRuntimeResult::ACCEPTED)return 3U;
 if(result==NotificationRuntimeResult::RETRY_LATER)return 2U;
 if(result==NotificationRuntimeResult::NO_CHANGE)return 1U;
 return 10U;
}
NotificationRuntimeResult combine(NotificationRuntimeResult current,NotificationRuntimeResult candidate)
{return updatePriority(candidate)>updatePriority(current)?candidate:current;}
}
NotificationRuntime::NotificationRuntime(NotificationRegistry& registry):registry_(registry),slots_{},slotCount_(0U),initialized_(false){}
size_t NotificationRuntime::findSlot(NotificationChannelId id)const
{for(size_t i=0U;i<slotCount_;++i)if(slots_[i].descriptor!=nullptr&&slots_[i].descriptor->channelId()==id)return i;return INVALID_SLOT;}
NotificationRuntimeResult NotificationRuntime::begin()
{
 for(size_t i=0U;i<slotCount_;++i)if(slots_[i].state.busy_)return NotificationRuntimeResult::CHANNEL_BUSY;
 initialized_=false;registry_.lock();const size_t count=registry_.size();
 if(count>NOTIFICATION_RUNTIME_MAX_CHANNELS)return NotificationRuntimeResult::INTERNAL_ERROR;
 NotificationRuntimeSlot prepared[NOTIFICATION_RUNTIME_MAX_CHANNELS];
 for(size_t i=0U;i<count;++i)
 {
  const NotificationChannelDescriptor* descriptor=registry_.descriptorAt(i);NotificationSink* sink=registry_.sinkAt(i);
  if(descriptor==nullptr||!descriptor->isValid()||sink==nullptr)return NotificationRuntimeResult::INTERNAL_ERROR;
  prepared[i].descriptor=descriptor;prepared[i].sink=sink;prepared[i].state.channelId_=descriptor->channelId();
  prepared[i].state.lastResult_=descriptor->enabled()?NotificationRuntimeResult::NO_CHANGE:NotificationRuntimeResult::CHANNEL_DISABLED;
  prepared[i].state.activeNotificationId_=0U;prepared[i].state.busy_=false;prepared[i].state.valid_=true;
 }
 for(size_t i=0U;i<NOTIFICATION_RUNTIME_MAX_CHANNELS;++i)slots_[i]=prepared[i];
 slotCount_=count;initialized_=true;return NotificationRuntimeResult::SUCCESS;
}
bool NotificationRuntime::isInitialized()const{return initialized_;}
NotificationRuntimeResult NotificationRuntime::applySendResult(NotificationRuntimeSlot& slot,NotificationId id,NotificationSinkResult result)
{
 NotificationRuntimeResult mapped=NotificationRuntimeResult::INTERNAL_ERROR;bool busy=false;
 if(isValidNotificationSinkResult(result))
 {
  if(result==NotificationSinkResult::SUCCESS)mapped=NotificationRuntimeResult::SUCCESS;
  else if(result==NotificationSinkResult::ACCEPTED){mapped=NotificationRuntimeResult::ACCEPTED;busy=true;}
  else if(result==NotificationSinkResult::IN_PROGRESS){mapped=NotificationRuntimeResult::IN_PROGRESS;busy=true;}
  else if(result==NotificationSinkResult::RETRY_LATER)mapped=NotificationRuntimeResult::RETRY_LATER;
  else if(result==NotificationSinkResult::REJECTED)mapped=NotificationRuntimeResult::SINK_REJECTED;
  else if(result==NotificationSinkResult::CANCELLED)mapped=NotificationRuntimeResult::CANCELLED;
  else if(result==NotificationSinkResult::FAILED)mapped=NotificationRuntimeResult::SINK_FAILED;
 }
 slot.state.lastResult_=mapped;slot.state.busy_=busy;slot.state.activeNotificationId_=busy?id:0U;
 return result==NotificationSinkResult::IN_PROGRESS?NotificationRuntimeResult::ACCEPTED:mapped;
}
NotificationRuntimeResult NotificationRuntime::send(const NotificationMessage& message,NotificationTimestamp now)
{
 if(!initialized_)return NotificationRuntimeResult::NOT_INITIALIZED;
 if(!message.isValid())return NotificationRuntimeResult::INVALID_NOTIFICATION;
 if(message.isExpired(now))return NotificationRuntimeResult::EXPIRED;
 const size_t index=findSlot(message.channelId());if(index==INVALID_SLOT)return NotificationRuntimeResult::CHANNEL_NOT_FOUND;
 NotificationRuntimeSlot& slot=slots_[index];if(!slot.descriptor->enabled())return NotificationRuntimeResult::CHANNEL_DISABLED;
 if(slot.state.busy_)return NotificationRuntimeResult::CHANNEL_BUSY;
 if(slot.sink==nullptr)return NotificationRuntimeResult::INTERNAL_ERROR;
 return applySendResult(slot,message.notificationId(),slot.sink->sendNotification(message,*slot.descriptor));
}
NotificationRuntimeResult NotificationRuntime::applyActiveResult(NotificationRuntimeSlot& slot,NotificationSinkResult result,bool cancelling)
{
 NotificationRuntimeResult mapped=NotificationRuntimeResult::INTERNAL_ERROR;bool keepBusy=false;
 if(isValidNotificationSinkResult(result))
 {
  if(result==NotificationSinkResult::SUCCESS)mapped=NotificationRuntimeResult::SUCCESS;
  else if(result==NotificationSinkResult::IN_PROGRESS){mapped=NotificationRuntimeResult::IN_PROGRESS;keepBusy=true;}
  else if(result==NotificationSinkResult::RETRY_LATER){mapped=NotificationRuntimeResult::RETRY_LATER;keepBusy=true;}
  else if(result==NotificationSinkResult::CANCELLED)mapped=NotificationRuntimeResult::CANCELLED;
  else if(result==NotificationSinkResult::REJECTED)mapped=NotificationRuntimeResult::SINK_REJECTED;
  else if(result==NotificationSinkResult::FAILED)mapped=NotificationRuntimeResult::SINK_FAILED;
  else if(result==NotificationSinkResult::ACCEPTED&&!cancelling){mapped=NotificationRuntimeResult::ACCEPTED;keepBusy=true;}
 }
 slot.state.lastResult_=mapped;slot.state.busy_=keepBusy;if(!keepBusy)slot.state.activeNotificationId_=0U;return mapped;
}
NotificationRuntimeResult NotificationRuntime::update(NotificationTimestamp now)
{
 (void)now;if(!initialized_)return NotificationRuntimeResult::NOT_INITIALIZED;
 NotificationRuntimeResult overall=NotificationRuntimeResult::NO_CHANGE;
 for(size_t i=0U;i<slotCount_;++i)
 {
  NotificationRuntimeSlot& slot=slots_[i];if(!slot.state.busy_)continue;
  NotificationRuntimeResult result=slot.sink==nullptr?NotificationRuntimeResult::INTERNAL_ERROR:applyActiveResult(slot,slot.sink->updateNotificationSend(slot.state.channelId_),false);
  if(slot.sink==nullptr){slot.state.lastResult_=result;slot.state.busy_=false;slot.state.activeNotificationId_=0U;}
  overall=combine(overall,result);
 }
 return overall;
}
NotificationRuntimeResult NotificationRuntime::cancel(NotificationChannelId channelId)
{
 if(!initialized_)return NotificationRuntimeResult::NOT_INITIALIZED;
 const size_t index=findSlot(channelId);if(index==INVALID_SLOT)return NotificationRuntimeResult::CHANNEL_NOT_FOUND;
 NotificationRuntimeSlot& slot=slots_[index];if(!slot.descriptor->enabled())return NotificationRuntimeResult::CHANNEL_DISABLED;
 if(!slot.state.busy_)return NotificationRuntimeResult::INVALID_ARGUMENT;
 if(slot.sink==nullptr){slot.state.lastResult_=NotificationRuntimeResult::INTERNAL_ERROR;slot.state.busy_=false;slot.state.activeNotificationId_=0U;return NotificationRuntimeResult::INTERNAL_ERROR;}
 return applyActiveResult(slot,slot.sink->cancelNotificationSend(channelId),true);
}
const NotificationRuntimeState* NotificationRuntime::state(NotificationChannelId id)const
{const size_t index=findSlot(id);return initialized_&&index!=INVALID_SLOT?&slots_[index].state:nullptr;}
bool NotificationRuntime::isBusy(NotificationChannelId id)const
{const NotificationRuntimeState* value=state(id);return value!=nullptr&&value->busy();}
