#include "RuleTriggerQueue.h"

RuleTriggerQueue::RuleTriggerQueue():triggers_{},count_(0){}
void RuleTriggerQueue::clear(){for(size_t i=0;i<RULE_TRIGGER_QUEUE_CAPACITY;++i)triggers_[i]=RuleTrigger{};count_=0U;}
RuleEngineResult RuleTriggerQueue::enqueue(const RuleTrigger& trigger)
{if(!trigger.isValid())return RuleEngineResult::NOT_FOUND;if(isFull())return RuleEngineResult::TRIGGER_QUEUE_FULL;triggers_[count_++]=trigger;return RuleEngineResult::SUCCESS;}
const RuleTrigger* RuleTriggerQueue::peek()const{return count_==0U?nullptr:&triggers_[0];}
RuleEngineResult RuleTriggerQueue::consume()
{if(isEmpty())return RuleEngineResult::QUEUE_EMPTY;for(size_t i=1U;i<count_;++i)triggers_[i-1U]=triggers_[i];--count_;triggers_[count_]=RuleTrigger{};return RuleEngineResult::SUCCESS;}
const RuleTrigger* RuleTriggerQueue::getAt(size_t index)const{return index<count_?&triggers_[index]:nullptr;}
size_t RuleTriggerQueue::size()const{return count_;}size_t RuleTriggerQueue::capacity()const{return RULE_TRIGGER_QUEUE_CAPACITY;}
bool RuleTriggerQueue::isFull()const{return count_>=RULE_TRIGGER_QUEUE_CAPACITY;}bool RuleTriggerQueue::isEmpty()const{return count_==0U;}
