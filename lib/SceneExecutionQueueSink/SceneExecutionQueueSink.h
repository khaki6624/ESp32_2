#ifndef SCENE_EXECUTION_QUEUE_SINK_H
#define SCENE_EXECUTION_QUEUE_SINK_H

#include <RuntimeCommandSink.h>
#include <SceneExecutionQueue.h>

class SceneExecutionQueueSink final : public RuntimeCommandSink
{
public:
    explicit SceneExecutionQueueSink(SceneExecutionQueue& queue);
    RuntimeCommandSubmitResult submit(const Command& command) override;
    bool isFull() const override;
private:
    SceneExecutionQueue& queue_;
};

#endif
