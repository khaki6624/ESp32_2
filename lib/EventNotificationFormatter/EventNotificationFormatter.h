#ifndef EVENT_NOTIFICATION_FORMATTER_H
#define EVENT_NOTIFICATION_FORMATTER_H
#include <RuntimeIntegrationCommon.h>
class EventNotificationFormatter{public:virtual ~EventNotificationFormatter()=default;virtual EventNotificationFormatResult format(const Event&,uint8_t*,size_t,size_t&)=0;};
#endif
