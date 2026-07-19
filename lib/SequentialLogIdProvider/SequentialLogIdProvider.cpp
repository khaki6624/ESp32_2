#include "SequentialLogIdProvider.h"

SequentialLogIdProvider::SequentialLogIdProvider(LogId initialValue) :
    next_(initialValue == INVALID_LOG_ID ? 1U : initialValue) {}

LogId SequentialLogIdProvider::nextLogId()
{
    const LogId result = next_;
    next_ = next_ == UINT32_MAX ? 1U : next_ + 1U;
    return result;
}
