#include "EventToLoggerAdapter.h"

EventToLoggerAdapter::EventToLoggerAdapter(LogSink& sink, LogIdProvider& logIdProvider) :
    sink_(sink), logIdProvider_(logIdProvider) {}

EventHandleResult EventToLoggerAdapter::handle(const Event& event)
{
    if (!event.isValid()) return EventHandleResult::FAILED;
    LogRecord record;
    if (!buildRecord(event, record)) return EventHandleResult::FAILED;
    return sink_.publish(record) == LogPublishResult::SUCCESS
        ? EventHandleResult::HANDLED : EventHandleResult::FAILED;
}

bool EventToLoggerAdapter::mapSource(
    EventSourceType eventSource, LogSourceType& logSource
) const
{
    switch (eventSource)
    {
        case EventSourceType::SYSTEM: logSource = LogSourceType::SYSTEM; return true;
        case EventSourceType::COMMAND: logSource = LogSourceType::COMMAND; return true;
        case EventSourceType::DEVICE: logSource = LogSourceType::DEVICE; return true;
        case EventSourceType::ENDPOINT: logSource = LogSourceType::ENDPOINT; return true;
        case EventSourceType::NODE: logSource = LogSourceType::NODE; return true;
        case EventSourceType::TRANSPORT: logSource = LogSourceType::TRANSPORT; return true;
        case EventSourceType::STORAGE: logSource = LogSourceType::STORAGE; return true;
        case EventSourceType::RULE: logSource = LogSourceType::RULE; return true;
        case EventSourceType::SCENE: logSource = LogSourceType::SCENE; return true;
        case EventSourceType::SCHEDULER: logSource = LogSourceType::SCHEDULER; return true;
        case EventSourceType::USER: logSource = LogSourceType::USER; return true;
        case EventSourceType::NONE:
        case EventSourceType::COUNT:
        default: return false;
    }
}

bool EventToLoggerAdapter::mapLevel(EventSeverity severity, LogLevel& level) const
{
    switch (severity)
    {
        case EventSeverity::INFO: level = LogLevel::INFO; return true;
        case EventSeverity::NOTICE: level = LogLevel::NOTICE; return true;
        case EventSeverity::WARNING: level = LogLevel::WARNING; return true;
        case EventSeverity::ERROR: level = LogLevel::ERROR; return true;
        case EventSeverity::CRITICAL: level = LogLevel::CRITICAL; return true;
        case EventSeverity::COUNT:
        default: return false;
    }
}

bool EventToLoggerAdapter::mapCategory(const Event& event, LogCategory& category) const
{
    if (event.type == EventType::COMMAND_REJECTED ||
        event.type == EventType::SAFETY_INTERLOCK_TRIGGERED ||
        event.type == EventType::CONFIRMATION_REQUIRED)
    {
        category = LogCategory::SECURITY;
        return true;
    }
    switch (event.sourceType)
    {
        case EventSourceType::SYSTEM: category = LogCategory::SYSTEM; return true;
        case EventSourceType::COMMAND: category = LogCategory::COMMAND; return true;
        case EventSourceType::DEVICE:
        case EventSourceType::ENDPOINT: category = LogCategory::DEVICE; return true;
        case EventSourceType::NODE: category = LogCategory::NODE; return true;
        case EventSourceType::STORAGE: category = LogCategory::STORAGE; return true;
        case EventSourceType::TRANSPORT: category = LogCategory::TRANSPORT; return true;
        case EventSourceType::RULE:
        case EventSourceType::SCENE:
        case EventSourceType::SCHEDULER: category = LogCategory::AUTOMATION; return true;
        case EventSourceType::USER: category = LogCategory::DIAGNOSTIC; return true;
        case EventSourceType::NONE:
        case EventSourceType::COUNT:
        default: return false;
    }
}

bool EventToLoggerAdapter::buildRecord(const Event& event, LogRecord& record)
{
    if (!mapSource(event.sourceType, record.sourceType) ||
        !mapLevel(event.severity, record.level) ||
        !mapCategory(event, record.category)) return false;

    record.timestampMs = event.timestampMs;
    record.sourceId = event.sourceId;
    record.code = static_cast<LogCode>(static_cast<uint16_t>(event.type));
    record.payload.value1 = event.payload.value1;
    record.payload.value2 = event.payload.value2;
    record.payload.signedValue = event.payload.signedValue;
    record.payload.flag = event.payload.flag;
    if (!record.setMessage(event.message)) return false;

    record.id = logIdProvider_.nextLogId();
    return record.id != INVALID_LOG_ID && record.isValid();
}
