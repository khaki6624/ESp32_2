#ifndef LOG_QUEUE_H
#define LOG_QUEUE_H

#include <LogSink.h>

class LogQueueTestAccess;

class LogQueue final : public LogSink
{
public:
    LogQueue();
    LogPublishResult publish(const LogRecord& record) override;
    const LogRecord* peek() const;
    LogQueueResult consume();
    void clear();
    size_t size() const;
    size_t capacity() const;
    bool empty() const;
    bool full() const;

private:
    // این دسترسی فقط برای ساخت فساد کنترل‌شده در تست است.
    friend class LogQueueTestAccess;
    LogRecord records_[LOG_QUEUE_CAPACITY];
    size_t head_;
    size_t tail_;
    size_t count_;
    bool contains(LogId id) const;
};

#endif
