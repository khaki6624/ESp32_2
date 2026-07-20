#include "NotificationRuntimeState.h"
NotificationRuntimeState::NotificationRuntimeState():channelId_(0U),lastResult_(NotificationRuntimeResult::NO_CHANGE),activeNotificationId_(0U),busy_(false),valid_(false){}
bool NotificationRuntimeState::isValid()const{return valid_&&channelId_!=0U&&isValidNotificationRuntimeResult(lastResult_)&&(!busy_||activeNotificationId_!=0U);}
NotificationChannelId NotificationRuntimeState::channelId()const{return channelId_;}
NotificationRuntimeResult NotificationRuntimeState::lastResult()const{return lastResult_;}
NotificationId NotificationRuntimeState::activeNotificationId()const{return activeNotificationId_;}
bool NotificationRuntimeState::busy()const{return busy_;}
