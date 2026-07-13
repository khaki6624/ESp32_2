#ifndef SCHEDULE_MANAGER_H
#define SCHEDULE_MANAGER_H
#include <Schedule.h>
class ScheduleManager
{
public:
    ScheduleManager();void clear();AutomationModelResult add(const Schedule&);AutomationModelResult update(const Schedule&);AutomationModelResult remove(ScheduleId);
    const Schedule* findById(ScheduleId)const;const Schedule* findByName(const char*)const;const Schedule* getAt(size_t)const;
    bool contains(ScheduleId)const;size_t size()const;size_t capacity()const;bool isFull()const;bool isEmpty()const;
private:Schedule schedules_[SCHEDULE_MANAGER_CAPACITY];size_t count_;size_t findIndexById(ScheduleId)const;bool nameBelongsToAnother(const char*,ScheduleId)const;
};
#endif
