#ifndef SCENE_EXECUTION_QUEUE_H
#define SCENE_EXECUTION_QUEUE_H

#include <AutomationExecutionCommon.h>
#include <Command.h>

class SceneExecutionQueue
{
public:
    SceneExecutionQueue();
    void clear();
    SceneExecutionResult enqueue(const Command& command);
    const Command* peek() const;
    SceneExecutionResult consume();
    const Command* getAt(size_t index) const;
    size_t size() const;size_t capacity() const;bool isFull() const;bool isEmpty() const;
private:
    Command commands_[SCENE_COMMAND_QUEUE_CAPACITY];
    size_t count_;
};

#endif
