#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <AutomationModelCommon.h>
#include <ScheduledCommand.h>

struct Schedule
{
    ScheduleId id;
    char name[AUTOMATION_NAME_MAX_LENGTH];
    ScheduledCommand commands[SCHEDULE_MAX_COMMANDS];
    uint8_t commandCount;
    bool enabled;
    Schedule():id(INVALID_SCHEDULE_ID),name{},commands{},commandCount(0),enabled(false){}
    bool isValid() const
    {
        if(id==INVALID_SCHEDULE_ID||name[0]=='\0'||!AutomationText::isCanonical(name,sizeof(name))||commandCount>SCHEDULE_MAX_COMMANDS)return false;
        for(size_t i=0;i<commandCount;++i){if(!commands[i].isValid()||commands[i].scheduleId!=id)return false;for(size_t j=i+1U;j<commandCount;++j)if(commands[i].commandIndex==commands[j].commandIndex)return false;}return true;
    }
    bool setName(const char* value){return value&&value[0]!='\0'&&AutomationText::set(name,sizeof(name),value);}
    AutomationModelResult addCommand(const ScheduledCommand& value)
    {
        if(!value.isValid())return AutomationModelResult::INVALID_COMMAND;
        if(value.scheduleId!=id)return AutomationModelResult::SCHEDULE_ID_MISMATCH;
        if(isFull())return AutomationModelResult::COMMAND_CAPACITY_FULL;
        if(findCommand(value.commandIndex))return AutomationModelResult::DUPLICATE_COMMAND_INDEX;
        // ترتیب Commands همان ترتیب فیزیکی افزودن است و Sort نمی‌شود.
        commands[commandCount++]=value;return AutomationModelResult::SUCCESS;
    }
    AutomationModelResult updateCommand(const ScheduledCommand& value)
    {if(!value.isValid())return AutomationModelResult::INVALID_COMMAND;if(value.scheduleId!=id)return AutomationModelResult::SCHEDULE_ID_MISMATCH;ScheduledCommand* p=findCommandMutable(value.commandIndex);if(!p)return AutomationModelResult::NOT_FOUND;*p=value;return AutomationModelResult::SUCCESS;}
    AutomationModelResult removeCommand(AutomationStepIndex commandIndex)
    {if(commandIndex==INVALID_AUTOMATION_STEP_INDEX)return AutomationModelResult::INVALID_INDEX;size_t i=0;while(i<commandCount&&commands[i].commandIndex!=commandIndex)++i;if(i==commandCount)return AutomationModelResult::NOT_FOUND;for(size_t j=i+1U;j<commandCount;++j)commands[j-1U]=commands[j];--commandCount;commands[commandCount]=ScheduledCommand{};return AutomationModelResult::SUCCESS;}
    const ScheduledCommand* findCommand(AutomationStepIndex index)const{for(size_t i=0;i<commandCount;++i)if(commands[i].commandIndex==index)return &commands[i];return nullptr;}
    const ScheduledCommand* getCommandAt(size_t index)const{return index<commandCount?&commands[index]:nullptr;}
    void clearCommands(){for(size_t i=0;i<SCHEDULE_MAX_COMMANDS;++i)commands[i]=ScheduledCommand{};commandCount=0;}
    void clear(){*this=Schedule{};} bool isEmpty()const{return commandCount==0U;} bool isFull()const{return commandCount>=SCHEDULE_MAX_COMMANDS;}
private:
    ScheduledCommand* findCommandMutable(AutomationStepIndex index){for(size_t i=0;i<commandCount;++i)if(commands[i].commandIndex==index)return &commands[i];return nullptr;}
};
#endif
