#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <EventHistoryRecord.h>

class EventManager
{
public:
    EventManager();
    void clear();
    bool shouldPersist(const Event& event) const;
    EventResultCode createHistoryRecord(const Event& event, EventHistoryRecord& record) const;
    EventResultCode enqueueHistory(const Event& event);
    const EventHistoryRecord* peekHistory() const;
    const EventHistoryRecord* getHistoryAt(size_t index) const;
    EventResultCode consumeHistory();
    EventResultCode consumeHistoryAt(size_t index);
    size_t historySize() const;
    size_t historyCapacity() const;
    bool isHistoryFull() const;
    bool isHistoryEmpty() const;
    static uint32_t calculateHistoryChecksum(const EventHistoryRecord& record);
    static bool validateHistoryRecord(const EventHistoryRecord& record);
    static EventPersistencePolicy defaultPolicyFor(EventType type);

private:
    EventHistoryRecord history_[EVENT_HISTORY_BUFFER_CAPACITY];
    size_t historyCount_;
};

#endif
