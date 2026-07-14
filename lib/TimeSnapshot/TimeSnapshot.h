#ifndef TIME_SNAPSHOT_H
#define TIME_SNAPSHOT_H

#include <AutomationCommon.h>

struct TimeSnapshot
{
    AutomationDate date;
    AutomationTime time;
    DayOfWeek dayOfWeek;
    uint32_t monotonicMs;
    bool valid;
    TimeSnapshot() : date{}, time{}, dayOfWeek(DayOfWeek::NONE), monotonicMs(0U), valid(false) {}
    bool isValid() const
    { return valid && date.isValid() && time.isValid() && dayOfWeek != DayOfWeek::NONE &&
             isValidDayOfWeek(dayOfWeek); }
    uint32_t secondsSinceMidnight() const
    { return static_cast<uint32_t>(time.hour) * 3600U +
             static_cast<uint32_t>(time.minute) * 60U + time.second; }
    void clear() { *this = TimeSnapshot{}; }
};

#endif
