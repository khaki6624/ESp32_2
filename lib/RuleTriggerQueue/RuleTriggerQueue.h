#ifndef RULE_TRIGGER_QUEUE_H
#define RULE_TRIGGER_QUEUE_H

#include <RuleEngineCommon.h>

class RuleTriggerQueue
{
public:
    RuleTriggerQueue();void clear();RuleEngineResult enqueue(const RuleTrigger& trigger);
    const RuleTrigger* peek()const;RuleEngineResult consume();const RuleTrigger* getAt(size_t index)const;
    size_t size()const;size_t capacity()const;bool isFull()const;bool isEmpty()const;
private:
    RuleTrigger triggers_[RULE_TRIGGER_QUEUE_CAPACITY];size_t count_;
};

#endif
