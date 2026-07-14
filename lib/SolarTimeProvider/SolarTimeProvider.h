#ifndef SOLAR_TIME_PROVIDER_H
#define SOLAR_TIME_PROVIDER_H

#include <SchedulerCommon.h>

class SolarTimeProvider
{
public:
    virtual ~SolarTimeProvider() = default;
    virtual SolarTimeProviderResult getSunrise(const AutomationDate& date,
        AutomationTime& output) const = 0;
    virtual SolarTimeProviderResult getSunset(const AutomationDate& date,
        AutomationTime& output) const = 0;
};

#endif
