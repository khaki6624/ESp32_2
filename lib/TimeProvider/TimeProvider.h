#ifndef TIME_PROVIDER_H
#define TIME_PROVIDER_H

#include <SchedulerCommon.h>
#include <TimeSnapshot.h>

class TimeProvider
{
public:
    virtual ~TimeProvider() = default;
    virtual TimeProviderResult getCurrentTime(TimeSnapshot& output) const = 0;
};

#endif
