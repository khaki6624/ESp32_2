#ifndef LOG_ID_PROVIDER_H
#define LOG_ID_PROVIDER_H

#include <LogCommon.h>

class LogIdProvider
{
public:
    virtual ~LogIdProvider() = default;
    virtual LogId nextLogId() = 0;
};

#endif
