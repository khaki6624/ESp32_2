#ifndef SERIAL_LOG_WRITER_H
#define SERIAL_LOG_WRITER_H

#include <LogRecordFormatter.h>
#include <LogWriter.h>
#include <TextOutput.h>

class SerialLogWriter final : public LogWriter
{
public:
    explicit SerialLogWriter(TextOutput& output);
    LogWriteResult write(const LogRecord& record) override;

private:
    TextOutput& output_;
    LogRecordFormatter formatter_;
    char buffer_[SERIAL_LOG_LINE_MAX_LENGTH];
};

#endif
