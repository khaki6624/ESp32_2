#ifndef AUTOMATION_EXECUTION_COMMON_H
#define AUTOMATION_EXECUTION_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <AutomationCommon.h>
#include <RequestContext.h>

constexpr size_t SCENE_REQUEST_QUEUE_CAPACITY = 4;
constexpr size_t SCENE_COMMAND_QUEUE_CAPACITY = 8;

enum class SceneExecutionState : uint8_t
{
    IDLE = 0, LOADING, READY_STEP, WAITING_COMMAND_QUEUE, WAITING_DELAY,
    COMPLETED, FAILED, CANCELLED
};

inline bool isValidSceneExecutionState(SceneExecutionState value)
{
    switch(value)
    {
        case SceneExecutionState::IDLE: case SceneExecutionState::LOADING:
        case SceneExecutionState::READY_STEP: case SceneExecutionState::WAITING_COMMAND_QUEUE:
        case SceneExecutionState::WAITING_DELAY: case SceneExecutionState::COMPLETED:
        case SceneExecutionState::FAILED: case SceneExecutionState::CANCELLED:return true;
        default:return false;
    }
}

inline bool isTerminalSceneExecutionState(SceneExecutionState value)
{return value==SceneExecutionState::COMPLETED||value==SceneExecutionState::FAILED||value==SceneExecutionState::CANCELLED;}

enum class SceneExecutionResult : uint8_t
{
    SUCCESS = 0, INVALID_SCENE_ID, INVALID_REQUEST_ID, INVALID_COMMAND_ID,
    INVALID_REQUEST_CONTEXT, SCENE_NOT_FOUND, SCENE_DISABLED, SCENE_INVALID,
    SCENE_EMPTY, REQUEST_QUEUE_FULL, COMMAND_QUEUE_FULL, COMMAND_QUEUE_EMPTY,
    ALREADY_RUNNING, NOT_RUNNING, REQUEST_NOT_FOUND, INVALID_STEP,
    COMMAND_FACTORY_FAILED, CANCELLED
};

inline bool isValidSceneExecutionResult(SceneExecutionResult value)
{
    switch(value)
    {
        case SceneExecutionResult::SUCCESS: case SceneExecutionResult::INVALID_SCENE_ID:
        case SceneExecutionResult::INVALID_REQUEST_ID: case SceneExecutionResult::INVALID_COMMAND_ID:
        case SceneExecutionResult::INVALID_REQUEST_CONTEXT: case SceneExecutionResult::SCENE_NOT_FOUND:
        case SceneExecutionResult::SCENE_DISABLED: case SceneExecutionResult::SCENE_INVALID:
        case SceneExecutionResult::SCENE_EMPTY: case SceneExecutionResult::REQUEST_QUEUE_FULL:
        case SceneExecutionResult::COMMAND_QUEUE_FULL: case SceneExecutionResult::COMMAND_QUEUE_EMPTY:
        case SceneExecutionResult::ALREADY_RUNNING: case SceneExecutionResult::NOT_RUNNING:
        case SceneExecutionResult::REQUEST_NOT_FOUND: case SceneExecutionResult::INVALID_STEP:
        case SceneExecutionResult::COMMAND_FACTORY_FAILED: case SceneExecutionResult::CANCELLED:return true;
        default:return false;
    }
}

inline bool isSceneExecutionSuccess(SceneExecutionResult value)
{return value==SceneExecutionResult::SUCCESS;}

enum class AutomationCommandFactoryResult : uint8_t
{
    SUCCESS = 0, INVALID_AUTOMATION_COMMAND, INVALID_REQUEST_CONTEXT,
    INVALID_COMMAND_ID, INVALID_REQUEST_ID, UNSUPPORTED_DOMAIN,
    UNSUPPORTED_OPERATION, ARGUMENT_CONVERSION_FAILED, OUTPUT_COMMAND_INVALID
};

inline bool isValidAutomationCommandFactoryResult(AutomationCommandFactoryResult value)
{
    switch(value)
    {
        case AutomationCommandFactoryResult::SUCCESS:
        case AutomationCommandFactoryResult::INVALID_AUTOMATION_COMMAND:
        case AutomationCommandFactoryResult::INVALID_REQUEST_CONTEXT:
        case AutomationCommandFactoryResult::INVALID_COMMAND_ID:
        case AutomationCommandFactoryResult::INVALID_REQUEST_ID:
        case AutomationCommandFactoryResult::UNSUPPORTED_DOMAIN:
        case AutomationCommandFactoryResult::UNSUPPORTED_OPERATION:
        case AutomationCommandFactoryResult::ARGUMENT_CONVERSION_FAILED:
        case AutomationCommandFactoryResult::OUTPUT_COMMAND_INVALID:return true;
        default:return false;
    }
}

inline bool isAutomationCommandFactorySuccess(AutomationCommandFactoryResult value)
{return value==AutomationCommandFactoryResult::SUCCESS;}

struct SceneExecutionRequest
{
    SceneId sceneId;
    RequestContext request;
    CommandId firstCommandId;
    uint32_t requestedTimestampMs;

    SceneExecutionRequest():sceneId(INVALID_SCENE_ID),request{},firstCommandId(INVALID_COMMAND_ID),requestedTimestampMs(0){}
    bool isValid() const
    {
        if(sceneId==INVALID_SCENE_ID||firstCommandId==INVALID_COMMAND_ID||!request.isValid())return false;
        switch(request.source)
        {
            case CommandSource::APP: case CommandSource::SMS: case CommandSource::RULE_ENGINE:
            case CommandSource::SCHEDULER: case CommandSource::SCENE: case CommandSource::SYSTEM:return true;
            default:return false;
        }
    }
};

#endif
