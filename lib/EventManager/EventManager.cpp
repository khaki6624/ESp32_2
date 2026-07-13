#include "EventManager.h"

#include <string.h>

namespace
{
void crcByte(uint32_t& crc, uint8_t value)
{
    crc ^= value;
    for (uint8_t bit=0;bit<8U;++bit) crc=(crc>>1U)^((crc&1U)?0xEDB88320UL:0U);
}
void crc16(uint32_t& crc,uint16_t value) { crcByte(crc,value&0xFFU); crcByte(crc,(value>>8U)&0xFFU); }
void crc32Value(uint32_t& crc,uint32_t value)
{
    for(uint8_t shift=0;shift<32U;shift+=8U) crcByte(crc,(value>>shift)&0xFFU);
}
void crcBuffer(uint32_t& crc,const char* value,size_t size)
{
    for(size_t i=0;i<size;++i) crcByte(crc,static_cast<uint8_t>(value[i]));
}
bool isCanonical(const char* value,size_t capacity)
{
    bool terminated=false;
    for(size_t i=0;i<capacity;++i)
    {
        if(value[i]=='\0') terminated=true;
        else if(terminated) return false;
    }
    return terminated;
}
}

EventManager::EventManager() : history_{}, historyCount_(0) {}
void EventManager::clear()
{
    for(size_t i=0;i<EVENT_HISTORY_BUFFER_CAPACITY;++i) history_[i]=EventHistoryRecord{};
    historyCount_=0;
}

bool EventManager::shouldPersist(const Event& event) const
{
    if(!event.isValid() || !event.isTerminal()) return false;
    if(event.persistencePolicy==EventPersistencePolicy::ALWAYS) return true;
    return event.persistencePolicy==EventPersistencePolicy::ON_FAILURE &&
           (event.status==EventStatus::FAILED || event.status==EventStatus::CANCELLED);
}

EventResultCode EventManager::createHistoryRecord(const Event& event, EventHistoryRecord& record) const
{
    if(!event.isValid()) return EventResultCode::INVALID_EVENT;
    if(!event.isTerminal()) return EventResultCode::NOT_TERMINAL;
    if(!shouldPersist(event)) return EventResultCode::HISTORY_BUFFER_INVALID;
    EventHistoryRecord temporary;
    temporary.eventId=event.id; temporary.correlationId=event.correlationId;
    temporary.type=event.type; temporary.severity=event.severity;
    temporary.sourceType=event.sourceType; temporary.sourceId=event.sourceId;
    temporary.createdTimestampMs=event.timestampMs; temporary.completedTimestampMs=event.completedMs;
    temporary.terminalStatus=event.status; temporary.resultCode=event.resultCode;
    // Copy مجدد، Byteهای پس از Null را Canonical و صفر می‌کند.
    if (!EventText::set(temporary.sourceName,sizeof(temporary.sourceName),event.sourceName) ||
        !EventText::set(temporary.message,sizeof(temporary.message),event.message))
        return EventResultCode::INVALID_EVENT;
    temporary.checksum=calculateHistoryChecksum(temporary);
    record=temporary;
    return EventResultCode::SUCCESS;
}

EventResultCode EventManager::enqueueHistory(const Event& event)
{
    EventHistoryRecord temporary;
    EventResultCode result=createHistoryRecord(event,temporary);
    if(result!=EventResultCode::SUCCESS) return result;
    if(isHistoryFull()) return EventResultCode::HISTORY_BUFFER_FULL;
    history_[historyCount_++]=temporary;
    return EventResultCode::SUCCESS;
}

const EventHistoryRecord* EventManager::peekHistory() const { return getHistoryAt(0); }
const EventHistoryRecord* EventManager::getHistoryAt(size_t index) const
{ return index<historyCount_?&history_[index]:nullptr; }
EventResultCode EventManager::consumeHistory() { return consumeHistoryAt(0); }
EventResultCode EventManager::consumeHistoryAt(size_t index)
{
    if(index>=historyCount_) return EventResultCode::NOT_FOUND;
    for(size_t i=index+1U;i<historyCount_;++i) history_[i-1U]=history_[i];
    --historyCount_; history_[historyCount_]=EventHistoryRecord{};
    return EventResultCode::SUCCESS;
}
size_t EventManager::historySize() const { return historyCount_; }
size_t EventManager::historyCapacity() const { return EVENT_HISTORY_BUFFER_CAPACITY; }
bool EventManager::isHistoryFull() const { return historyCount_>=EVENT_HISTORY_BUFFER_CAPACITY; }
bool EventManager::isHistoryEmpty() const { return historyCount_==0; }

uint32_t EventManager::calculateHistoryChecksum(const EventHistoryRecord& r)
{
    uint32_t crc=0xFFFFFFFFUL;
    crc16(crc,r.version); crc32Value(crc,r.eventId); crc32Value(crc,r.correlationId);
    crc16(crc,static_cast<uint16_t>(r.type)); crcByte(crc,static_cast<uint8_t>(r.severity));
    crcByte(crc,static_cast<uint8_t>(r.sourceType)); crc32Value(crc,r.sourceId);
    crc32Value(crc,r.createdTimestampMs); crc32Value(crc,r.completedTimestampMs);
    crcByte(crc,static_cast<uint8_t>(r.terminalStatus));
    crc16(crc,static_cast<uint16_t>(r.resultCode));
    crcBuffer(crc,r.sourceName,sizeof(r.sourceName)); crcBuffer(crc,r.message,sizeof(r.message));
    return crc^0xFFFFFFFFUL;
}

bool EventManager::validateHistoryRecord(const EventHistoryRecord& record)
{
    return record.isValid() && isCanonical(record.sourceName,sizeof(record.sourceName)) &&
           isCanonical(record.message,sizeof(record.message)) &&
           record.checksum==calculateHistoryChecksum(record);
}

EventPersistencePolicy EventManager::defaultPolicyFor(EventType type)
{
    switch(type)
    {
        case EventType::COMMAND_REJECTED: case EventType::COMMAND_FAILED:
        case EventType::SAFETY_INTERLOCK_TRIGGERED: case EventType::NODE_OFFLINE:
        case EventType::SYSTEM_REBOOTED: case EventType::STORAGE_ERROR:
            return EventPersistencePolicy::ALWAYS;
        case EventType::COMMUNICATION_ERROR: return EventPersistencePolicy::ON_FAILURE;
        default: return EventPersistencePolicy::NONE;
    }
}
