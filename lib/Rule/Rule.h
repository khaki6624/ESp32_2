#ifndef RULE_H
#define RULE_H

#include <AutomationModelCommon.h>
#include <ConditionExpression.h>
#include <RuleActionStep.h>

struct Rule
{
    RuleId id; char name[AUTOMATION_NAME_MAX_LENGTH]; ConditionExpression condition;
    RuleActionStep thenSteps[RULE_MAX_ACTION_STEPS]; uint8_t thenStepCount;
    RuleActionStep elseSteps[RULE_MAX_ACTION_STEPS]; uint8_t elseStepCount; bool enabled;
    Rule():id(INVALID_RULE_ID),name{},condition{},thenSteps{},thenStepCount(0),elseSteps{},elseStepCount(0),enabled(false){}
    bool isValid()const
    {
        if(id==INVALID_RULE_ID||name[0]=='\0'||!AutomationText::isCanonical(name,sizeof(name))||!condition.isValid()||thenStepCount>RULE_MAX_ACTION_STEPS||elseStepCount>RULE_MAX_ACTION_STEPS)return false;
        for(size_t i=0;i<thenStepCount;++i){if(!thenSteps[i].isValid()||thenSteps[i].branch!=RuleBranch::THEN_BRANCH)return false;for(size_t j=i+1U;j<thenStepCount;++j)if(thenSteps[i].stepIndex==thenSteps[j].stepIndex)return false;}
        for(size_t i=0;i<elseStepCount;++i){if(!elseSteps[i].isValid()||elseSteps[i].branch!=RuleBranch::ELSE_BRANCH)return false;for(size_t j=i+1U;j<elseStepCount;++j)if(elseSteps[i].stepIndex==elseSteps[j].stepIndex)return false;}return true;
    }
    bool setName(const char* value){return value&&value[0]!='\0'&&AutomationText::set(name,sizeof(name),value);}
    bool setCondition(const ConditionExpression& value){if(!value.isValid())return false;condition=value;return true;}
    AutomationModelResult addActionStep(const RuleActionStep& step)
    {if(!step.isValid())return AutomationModelResult::INVALID_STEP;if(step.branch!=RuleBranch::THEN_BRANCH&&step.branch!=RuleBranch::ELSE_BRANCH)return AutomationModelResult::BRANCH_MISMATCH;uint8_t& count=step.branch==RuleBranch::THEN_BRANCH?thenStepCount:elseStepCount;RuleActionStep* array=step.branch==RuleBranch::THEN_BRANCH?thenSteps:elseSteps;if(count>=RULE_MAX_ACTION_STEPS)return AutomationModelResult::STEP_CAPACITY_FULL;for(size_t i=0;i<count;++i)if(array[i].stepIndex==step.stepIndex)return AutomationModelResult::DUPLICATE_STEP_INDEX;array[count++]=step;return AutomationModelResult::SUCCESS;}
    AutomationModelResult updateActionStep(const RuleActionStep& step)
    {
        if(step.branch!=RuleBranch::THEN_BRANCH&&step.branch!=RuleBranch::ELSE_BRANCH)return AutomationModelResult::BRANCH_MISMATCH;
        if(!step.isValid())return AutomationModelResult::INVALID_STEP;
        RuleActionStep* current=findActionStepMutable(step.branch,step.stepIndex);
        if(current==nullptr)return AutomationModelResult::NOT_FOUND;
        *current=step;
        // انتقال Step میان THEN و ELSE با update انجام نمی‌شود. Caller باید ابتدا
        // remove و سپس add کند؛ در صورت نیاز آینده API مستقل moveActionStep طراحی می‌شود.
        return AutomationModelResult::SUCCESS;
    }
    AutomationModelResult removeActionStep(RuleBranch branch,AutomationStepIndex index)
    {if(branch!=RuleBranch::THEN_BRANCH&&branch!=RuleBranch::ELSE_BRANCH)return AutomationModelResult::BRANCH_MISMATCH;if(index==INVALID_AUTOMATION_STEP_INDEX)return AutomationModelResult::INVALID_INDEX;uint8_t& count=branch==RuleBranch::THEN_BRANCH?thenStepCount:elseStepCount;RuleActionStep* a=branch==RuleBranch::THEN_BRANCH?thenSteps:elseSteps;size_t i=0;while(i<count&&a[i].stepIndex!=index)++i;if(i==count)return AutomationModelResult::NOT_FOUND;for(size_t j=i+1U;j<count;++j)a[j-1U]=a[j];--count;a[count]=RuleActionStep{};return AutomationModelResult::SUCCESS;}
    const RuleActionStep* findActionStep(RuleBranch branch,AutomationStepIndex index)const{uint8_t count=branch==RuleBranch::THEN_BRANCH?thenStepCount:branch==RuleBranch::ELSE_BRANCH?elseStepCount:0;const RuleActionStep* a=branch==RuleBranch::THEN_BRANCH?thenSteps:branch==RuleBranch::ELSE_BRANCH?elseSteps:nullptr;for(size_t i=0;i<count;++i)if(a[i].stepIndex==index)return &a[i];return nullptr;}
    const RuleActionStep* getActionStepAt(RuleBranch b,size_t i)const{return b==RuleBranch::THEN_BRANCH&&i<thenStepCount?&thenSteps[i]:b==RuleBranch::ELSE_BRANCH&&i<elseStepCount?&elseSteps[i]:nullptr;}
    size_t actionCount(RuleBranch b)const{return b==RuleBranch::THEN_BRANCH?thenStepCount:b==RuleBranch::ELSE_BRANCH?elseStepCount:0U;}
    void clearActions(RuleBranch b){if(b==RuleBranch::THEN_BRANCH){for(size_t i=0;i<RULE_MAX_ACTION_STEPS;++i)thenSteps[i]=RuleActionStep{};thenStepCount=0;}else if(b==RuleBranch::ELSE_BRANCH){for(size_t i=0;i<RULE_MAX_ACTION_STEPS;++i)elseSteps[i]=RuleActionStep{};elseStepCount=0;}}
    void clearAllActions(){clearActions(RuleBranch::THEN_BRANCH);clearActions(RuleBranch::ELSE_BRANCH);} void clear(){*this=Rule{};}
private:
    RuleActionStep* findActionStepMutable(RuleBranch branch,AutomationStepIndex index){uint8_t count=branch==RuleBranch::THEN_BRANCH?thenStepCount:branch==RuleBranch::ELSE_BRANCH?elseStepCount:0;RuleActionStep* a=branch==RuleBranch::THEN_BRANCH?thenSteps:branch==RuleBranch::ELSE_BRANCH?elseSteps:nullptr;for(size_t i=0;i<count;++i)if(a[i].stepIndex==index)return &a[i];return nullptr;}
};
#endif
