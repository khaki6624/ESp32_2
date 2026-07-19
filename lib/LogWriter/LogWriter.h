#ifndef LOG_WRITER_H
#define LOG_WRITER_H

#include <LogRecord.h>

class LogWriter
{
public:
    virtual ~LogWriter() = default;
    virtual LogWriteResult write(const LogRecord& record) = 0;
};

#endif
