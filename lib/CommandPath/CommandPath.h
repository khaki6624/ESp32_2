#ifndef COMMAND_PATH_H
#define COMMAND_PATH_H
#include <CommandCommon.h>

struct CommandPathSegment
{
    char name[COMMAND_PATH_SEGMENT_MAX_LENGTH];
    uint16_t index;
    bool hasIndex;
    CommandPathSegment() : name{},index(0),hasIndex(false) {}
    bool isValid() const
    { return name[0]!='\0' && CommandText::isTerminated(name,sizeof(name)) && (!hasIndex||index!=0); }
    bool setName(const char* value)
    { if(value==nullptr||value[0]=='\0') return false; return CommandText::set(name,sizeof(name),value); }
};

struct CommandPath
{
    CommandPathSegment segments[COMMAND_PATH_MAX_SEGMENTS];
    uint8_t count;
    CommandPath() : segments{},count(0) {}
    void clear() { for(size_t i=0;i<COMMAND_PATH_MAX_SEGMENTS;++i) segments[i]=CommandPathSegment{}; count=0; }
    bool isValid() const
    { if(count>COMMAND_PATH_MAX_SEGMENTS) return false; for(size_t i=0;i<count;++i) if(!segments[i].isValid()) return false; return true; }
    bool isEmpty() const { return count==0; }
    bool isFull() const { return count>=COMMAND_PATH_MAX_SEGMENTS; }
    bool addSegment(const char* value,uint16_t valueIndex=0,bool valueHasIndex=false)
    {
        if(isFull()||(valueHasIndex&&valueIndex==0)) return false;
        CommandPathSegment temporary;
        if(!temporary.setName(value)) return false;
        temporary.index=valueHasIndex?valueIndex:0; temporary.hasIndex=valueHasIndex;
        if(!temporary.isValid()) return false;
        segments[count++]=temporary; return true;
    }
    const CommandPathSegment* getAt(size_t valueIndex) const
    { return valueIndex<count?&segments[valueIndex]:nullptr; }
};
#endif
