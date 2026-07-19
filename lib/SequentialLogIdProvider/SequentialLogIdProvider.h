#ifndef SEQUENTIAL_LOG_ID_PROVIDER_H
#define SEQUENTIAL_LOG_ID_PROVIDER_H

#include <LogIdProvider.h>

class SequentialLogIdProvider final : public LogIdProvider
{
public:
    explicit SequentialLogIdProvider(LogId initialValue = 1U);
    LogId nextLogId() override;

private:
    LogId next_;
};

#endif
