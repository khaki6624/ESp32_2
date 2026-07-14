#include "SceneExecutionQueueSink.h"

SceneExecutionQueueSink::SceneExecutionQueueSink(SceneExecutionQueue& queue) : queue_(queue) {}

RuntimeCommandSubmitResult SceneExecutionQueueSink::submit(const Command& command)
{
    // اعتبارسنجی زودهنگام، Queue را در خطا بدون تغییر نگه می‌دارد.
    if (!command.isValid()) return RuntimeCommandSubmitResult::INVALID_COMMAND;
    switch (queue_.enqueue(command))
    {
        case SceneExecutionResult::SUCCESS: return RuntimeCommandSubmitResult::SUCCESS;
        case SceneExecutionResult::COMMAND_QUEUE_FULL: return RuntimeCommandSubmitResult::SINK_FULL;
        case SceneExecutionResult::COMMAND_FACTORY_FAILED:
            return RuntimeCommandSubmitResult::INVALID_COMMAND;
        default: return RuntimeCommandSubmitResult::SINK_ERROR;
    }
}

bool SceneExecutionQueueSink::isFull() const { return queue_.isFull(); }
