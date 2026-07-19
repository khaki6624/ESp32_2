#include "LoggerRuntime.h"

LoggerRuntime::LoggerRuntime(LogQueue& queue) :
    queue_(queue), writers_{}, writerCount_(0U), processing_(false) {}

LogWriterRegistrationResult LoggerRuntime::addWriter(LogWriter& writer)
{
    for (size_t index = 0U; index < writerCount_; ++index)
        if (writers_[index] == &writer) return LogWriterRegistrationResult::DUPLICATE_WRITER;
    if (writerCount_ == LOG_WRITER_CAPACITY) return LogWriterRegistrationResult::REGISTRY_FULL;
    writers_[writerCount_++] = &writer;
    return LogWriterRegistrationResult::SUCCESS;
}

LogWriterRegistrationResult LoggerRuntime::removeWriter(LogWriter& writer)
{
    for (size_t index = 0U; index < writerCount_; ++index)
    {
        if (writers_[index] != &writer) continue;
        for (size_t shift = index + 1U; shift < writerCount_; ++shift)
            writers_[shift - 1U] = writers_[shift];
        --writerCount_;
        writers_[writerCount_] = nullptr;
        return LogWriterRegistrationResult::SUCCESS;
    }
    return LogWriterRegistrationResult::WRITER_NOT_FOUND;
}

LoggerUpdateResult LoggerRuntime::update()
{
    if (processing_) return LoggerUpdateResult::REENTRANT_CALL;
    const LogRecord* record = queue_.peek();
    if (record == nullptr) return LoggerUpdateResult::QUEUE_EMPTY;
    if (!record->isValid())
        return queue_.consume() == LogQueueResult::SUCCESS
            ? LoggerUpdateResult::INVALID_RECORD : LoggerUpdateResult::INTERNAL_ERROR;

    bool written = false;
    bool failed = false;
    bool retryLater = false;
    processing_ = true;
    for (size_t index = 0U; index < writerCount_; ++index)
    {
        LogWriter* const writer = writers_[index];
        if (writer == nullptr) { failed = true; continue; }
        const LogWriteResult result = writer->write(*record);
        if (!isValidLogWriteResult(result) || result == LogWriteResult::FAILED) failed = true;
        else if (result == LogWriteResult::RETRY_LATER) retryLater = true;
        else if (result == LogWriteResult::WRITTEN) written = true;
    }

    LoggerUpdateResult result;
    if (retryLater)
        result = LoggerUpdateResult::RETRY_LATER;
    else if (queue_.consume() != LogQueueResult::SUCCESS)
        result = LoggerUpdateResult::INTERNAL_ERROR;
    else if (failed)
        result = LoggerUpdateResult::WRITER_FAILED;
    else if (writerCount_ == 0U)
        result = LoggerUpdateResult::NO_WRITERS;
    else
        result = written ? LoggerUpdateResult::SUCCESS : LoggerUpdateResult::RECORD_IGNORED;
    processing_ = false;
    return result;
}

size_t LoggerRuntime::writerCount() const { return writerCount_; }
size_t LoggerRuntime::writerCapacity() const { return LOG_WRITER_CAPACITY; }
