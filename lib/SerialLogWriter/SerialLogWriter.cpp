#include "SerialLogWriter.h"

SerialLogWriter::SerialLogWriter(TextOutput& output) :
    output_(output), formatter_{}, buffer_{} {}

LogWriteResult SerialLogWriter::write(const LogRecord& record)
{
    size_t length = 0U;
    if (formatter_.format(record, buffer_, sizeof(buffer_), length) != LogFormatResult::SUCCESS)
        return LogWriteResult::FAILED;
    const TextOutputResult result = output_.write(buffer_, length);
    if (result == TextOutputResult::SUCCESS) return LogWriteResult::WRITTEN;
    if (result == TextOutputResult::RETRY_LATER) return LogWriteResult::RETRY_LATER;
    return LogWriteResult::FAILED;
}
