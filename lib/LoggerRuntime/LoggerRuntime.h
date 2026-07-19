#ifndef LOGGER_RUNTIME_H
#define LOGGER_RUNTIME_H

#include <LogQueue.h>
#include <LogWriter.h>

class LoggerRuntime
{
public:
    explicit LoggerRuntime(LogQueue& queue);
    LogWriterRegistrationResult addWriter(LogWriter& writer);
    LogWriterRegistrationResult removeWriter(LogWriter& writer);
    LoggerUpdateResult update();
    size_t writerCount() const;
    size_t writerCapacity() const;

private:
    LogQueue& queue_;
    // مالکیت Writerها نزد فراخواننده است و طول عمرشان باید از Registration بیشتر باشد.
    LogWriter* writers_[LOG_WRITER_CAPACITY];
    size_t writerCount_;
    bool processing_;
};

#endif
