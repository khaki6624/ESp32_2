#include "SceneExecutionQueue.h"

SceneExecutionQueue::SceneExecutionQueue():commands_{},count_(0){}
void SceneExecutionQueue::clear(){for(size_t i=0;i<SCENE_COMMAND_QUEUE_CAPACITY;++i)commands_[i]=Command{};count_=0;}
SceneExecutionResult SceneExecutionQueue::enqueue(const Command& command)
{if(!command.isValid())return SceneExecutionResult::COMMAND_FACTORY_FAILED;if(isFull())return SceneExecutionResult::COMMAND_QUEUE_FULL;commands_[count_++]=command;return SceneExecutionResult::SUCCESS;}
const Command* SceneExecutionQueue::peek()const{return count_==0U?nullptr:&commands_[0];}
SceneExecutionResult SceneExecutionQueue::consume()
{if(isEmpty())return SceneExecutionResult::COMMAND_QUEUE_EMPTY;for(size_t i=1;i<count_;++i)commands_[i-1U]=commands_[i];--count_;commands_[count_]=Command{};return SceneExecutionResult::SUCCESS;}
const Command* SceneExecutionQueue::getAt(size_t index)const{return index<count_?&commands_[index]:nullptr;}
size_t SceneExecutionQueue::size()const{return count_;}size_t SceneExecutionQueue::capacity()const{return SCENE_COMMAND_QUEUE_CAPACITY;}
bool SceneExecutionQueue::isFull()const{return count_>=SCENE_COMMAND_QUEUE_CAPACITY;}bool SceneExecutionQueue::isEmpty()const{return count_==0U;}
