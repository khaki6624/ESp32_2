#ifndef NOTIFICATION_MESSAGE_H
#define NOTIFICATION_MESSAGE_H

#include <NotificationRuntimeCommon.h>

// Non-owning view. The caller keeps target/payload alive through send(); after
// acceptance, a sink is responsible for any storage it requires.
class NotificationMessage
{
public:
 NotificationMessage();
 static NotificationMessage create(NotificationId notificationId,
  NotificationChannelId channelId, NotificationPriority priority,
  NotificationTimestamp createdAt, NotificationDuration ttlMs,
  const char* target, size_t targetLength, const uint8_t* payload,
  size_t payloadLength);
 bool isValid() const;
 NotificationId notificationId() const;
 NotificationChannelId channelId() const;
 NotificationPriority priority() const;
 NotificationTimestamp createdAt() const;
 NotificationDuration ttlMs() const;
 const char* target() const;
 size_t targetLength() const;
 const uint8_t* payload() const;
 size_t payloadLength() const;
 bool isExpired(NotificationTimestamp now) const;
private:
 NotificationId notificationId_;
 NotificationChannelId channelId_;
 NotificationPriority priority_;
 NotificationTimestamp createdAt_;
 NotificationDuration ttlMs_;
 const char* target_;
 size_t targetLength_;
 const uint8_t* payload_;
 size_t payloadLength_;
 bool valid_;
};

#endif
