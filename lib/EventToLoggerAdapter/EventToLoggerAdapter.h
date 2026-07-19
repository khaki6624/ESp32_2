#ifndef EVENT_TO_LOGGER_ADAPTER_H
#define EVENT_TO_LOGGER_ADAPTER_H

#include <EventHandler.h>
#include <LogIdProvider.h>
#include <LogSink.h>

class EventToLoggerAdapter final : public EventHandler
{
public:
    EventToLoggerAdapter(LogSink& sink, LogIdProvider& logIdProvider);
    EventHandleResult handle(const Event& event) override;

private:
    LogSink& sink_;
    LogIdProvider& logIdProvider_;

    bool mapSource(EventSourceType eventSource, LogSourceType& logSource) const;
    bool mapLevel(EventSeverity severity, LogLevel& level) const;
    bool mapCategory(const Event& event, LogCategory& category) const;
    bool buildRecord(const Event& event, LogRecord& record);
};

#endif
