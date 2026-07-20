#include "NotificationRegistry.h"
NotificationRegistry::NotificationRegistry():descriptors_{},sinks_{},size_(0U),locked_(false){}
NotificationRuntimeResult NotificationRegistry::registerChannel(const NotificationChannelDescriptor& descriptor,NotificationSink& sink)
{
 if(locked_)return NotificationRuntimeResult::REGISTRY_LOCKED;
 if(!descriptor.isValid())return NotificationRuntimeResult::INVALID_CHANNEL;
 for(size_t i=0U;i<size_;++i)if(descriptors_[i].channelId()==descriptor.channelId())return NotificationRuntimeResult::DUPLICATE_CHANNEL_ID;
 if(size_>=NOTIFICATION_RUNTIME_MAX_CHANNELS)return NotificationRuntimeResult::REGISTRY_FULL;
 descriptors_[size_]=descriptor;sinks_[size_]=&sink;++size_;return NotificationRuntimeResult::SUCCESS;
}
const NotificationChannelDescriptor* NotificationRegistry::findDescriptor(NotificationChannelId id)const
{if(id==0U)return nullptr;for(size_t i=0U;i<size_;++i)if(descriptors_[i].channelId()==id)return &descriptors_[i];return nullptr;}
NotificationSink* NotificationRegistry::findSink(NotificationChannelId id)const
{if(id==0U)return nullptr;for(size_t i=0U;i<size_;++i)if(descriptors_[i].channelId()==id)return sinks_[i];return nullptr;}
const NotificationChannelDescriptor* NotificationRegistry::descriptorAt(size_t i)const{return i<size_?&descriptors_[i]:nullptr;}
NotificationSink* NotificationRegistry::sinkAt(size_t i)const{return i<size_?sinks_[i]:nullptr;}
size_t NotificationRegistry::size()const{return size_;}size_t NotificationRegistry::capacity()const{return NOTIFICATION_RUNTIME_MAX_CHANNELS;}
bool NotificationRegistry::isLocked()const{return locked_;}NotificationRuntimeResult NotificationRegistry::lock(){locked_=true;return NotificationRuntimeResult::SUCCESS;}
