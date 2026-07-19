#include "LogQueue.h"

LogQueue::LogQueue() : records_{}, head_(0U), tail_(0U), count_(0U) {}

LogPublishResult LogQueue::publish(const LogRecord& record)
{
    if (!record.isValid()) return LogPublishResult::INVALID_RECORD;
    if (full()) return LogPublishResult::QUEUE_FULL;
    if (contains(record.id)) return LogPublishResult::DUPLICATE_LOG_ID;
    records_[tail_] = record;
    tail_ = (tail_ + 1U) % LOG_QUEUE_CAPACITY;
    ++count_;
    return LogPublishResult::SUCCESS;
}

const LogRecord* LogQueue::peek() const
{ return empty() ? nullptr : &records_[head_]; }

LogQueueResult LogQueue::consume()
{
    if (empty()) return LogQueueResult::QUEUE_EMPTY;
    records_[head_] = LogRecord{};
    head_ = (head_ + 1U) % LOG_QUEUE_CAPACITY;
    --count_;
    return LogQueueResult::SUCCESS;
}

void LogQueue::clear()
{
    for (size_t index = 0U; index < LOG_QUEUE_CAPACITY; ++index)
        records_[index] = LogRecord{};
    head_ = 0U;
    tail_ = 0U;
    count_ = 0U;
}

size_t LogQueue::size() const { return count_; }
size_t LogQueue::capacity() const { return LOG_QUEUE_CAPACITY; }
bool LogQueue::empty() const { return count_ == 0U; }
bool LogQueue::full() const { return count_ == LOG_QUEUE_CAPACITY; }

bool LogQueue::contains(LogId id) const
{
    for (size_t offset = 0U; offset < count_; ++offset)
        if (records_[(head_ + offset) % LOG_QUEUE_CAPACITY].id == id) return true;
    return false;
}
