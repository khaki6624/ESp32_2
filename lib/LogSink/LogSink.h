#ifndef LOG_SINK_H
#define LOG_SINK_H

#include <LogRecord.h>

class LogSink
{
public:
    virtual ~LogSink() = default;
    virtual LogPublishResult publish(const LogRecord& record) = 0;
};

#endif
