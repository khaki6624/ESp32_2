#ifndef NOTIFICATION_RUNTIME_COMMON_H
#define NOTIFICATION_RUNTIME_COMMON_H

#include <stddef.h>
#include <stdint.h>

using NotificationId = uint32_t;
using NotificationChannelId = uint8_t;
using NotificationTimestamp = uint32_t;
using NotificationDuration = uint32_t;

constexpr NotificationId INVALID_NOTIFICATION_ID = 0U;
constexpr NotificationChannelId INVALID_NOTIFICATION_CHANNEL_ID = 0U;
constexpr size_t NOTIFICATION_RUNTIME_MAX_CHANNELS = 8U;
constexpr size_t NOTIFICATION_RUNTIME_MAX_PAYLOAD_LENGTH = 256U;
constexpr size_t NOTIFICATION_RUNTIME_MAX_TARGET_LENGTH = 48U;

// LOW/HIGH are Arduino macros, so the public enumerators remain macro-safe.
enum class NotificationPriority : uint8_t { LOW_PRIORITY = 0, NORMAL, HIGH_PRIORITY, CRITICAL, COUNT };
enum class NotificationChannelType : uint8_t { GENERIC = 0, SMS, PUSH, EMAIL, APP, COUNT };
enum class NotificationSinkResult : uint8_t
{ SUCCESS = 0, ACCEPTED, IN_PROGRESS, RETRY_LATER, REJECTED, CANCELLED, FAILED, COUNT };
enum class NotificationRuntimeResult : uint8_t
{
 SUCCESS = 0, ACCEPTED, IN_PROGRESS, RETRY_LATER, NO_CHANGE,
 CANCELLED, EXPIRED, SINK_REJECTED, SINK_FAILED, INTERNAL_ERROR,
 NOT_INITIALIZED, INVALID_ARGUMENT, INVALID_NOTIFICATION, INVALID_CHANNEL,
 CHANNEL_NOT_FOUND, CHANNEL_DISABLED, CHANNEL_BUSY, REGISTRY_FULL,
 REGISTRY_LOCKED, DUPLICATE_CHANNEL_ID, COUNT
};

inline bool isValidNotificationPriority(NotificationPriority value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(NotificationPriority::COUNT); }
inline bool isValidNotificationChannelType(NotificationChannelType value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(NotificationChannelType::COUNT); }
inline bool isValidNotificationSinkResult(NotificationSinkResult value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(NotificationSinkResult::COUNT); }
inline bool isValidNotificationRuntimeResult(NotificationRuntimeResult value)
{ return static_cast<uint8_t>(value) < static_cast<uint8_t>(NotificationRuntimeResult::COUNT); }

#endif
