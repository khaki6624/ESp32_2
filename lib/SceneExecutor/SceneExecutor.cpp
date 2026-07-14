#include "SceneExecutor.h"

#include <stdint.h>

SceneExecutor::SceneExecutor(const SceneManager& sceneManager,
    const AutomationCommandFactory& commandFactory,SceneExecutionQueue& commandQueue):
    sceneManager_(sceneManager),commandFactory_(commandFactory),commandQueue_(commandQueue),
    requests_{},requestCount_(0),currentRequest_{},state_(SceneExecutionState::IDLE),
    lastResult_(SceneExecutionResult::SUCCESS),currentSceneId_(INVALID_SCENE_ID),
    currentStepArrayIndex_(0),nextCommandId_(INVALID_COMMAND_ID),delayStartedMs_(0),delayDurationMs_(0){}

SceneExecutionResult SceneExecutor::validateRequest(const SceneExecutionRequest& request)const
{
    if(request.sceneId==INVALID_SCENE_ID)return SceneExecutionResult::INVALID_SCENE_ID;
    if(request.request.requestId==INVALID_REQUEST_ID)return SceneExecutionResult::INVALID_REQUEST_ID;
    if(request.firstCommandId==INVALID_COMMAND_ID)return SceneExecutionResult::INVALID_COMMAND_ID;
    if(!request.request.isValid()||!request.isValid())return SceneExecutionResult::INVALID_REQUEST_CONTEXT;
    return SceneExecutionResult::SUCCESS;
}

SceneExecutionResult SceneExecutor::validateScene(const SceneExecutionRequest& request)const
{
    const Scene* scene=sceneManager_.findById(request.sceneId);
    if(scene==nullptr)return SceneExecutionResult::SCENE_NOT_FOUND;
    if(!scene->isValid())return SceneExecutionResult::SCENE_INVALID;
    if(!scene->enabled)return SceneExecutionResult::SCENE_DISABLED;
    if(scene->isEmpty())return SceneExecutionResult::SCENE_EMPTY;
    return SceneExecutionResult::SUCCESS;
}

bool SceneExecutor::hasValidCommandIdRange(const SceneExecutionRequest& request,const Scene& scene)const
{
    if(scene.stepCount==0U||request.firstCommandId==INVALID_COMMAND_ID)return false;
    const uint32_t requiredIncrement=static_cast<uint32_t>(scene.stepCount-1U);
    return request.firstCommandId<=UINT32_MAX-requiredIncrement;
}

SceneExecutionResult SceneExecutor::enqueue(const SceneExecutionRequest& request)
{
    const SceneExecutionResult validation=validateRequest(request);
    if(validation!=SceneExecutionResult::SUCCESS)return validation;
    if(requestCount_>=SCENE_REQUEST_QUEUE_CAPACITY)return SceneExecutionResult::REQUEST_QUEUE_FULL;
    requests_[requestCount_++]=request;
    return SceneExecutionResult::SUCCESS;
}

SceneExecutionResult SceneExecutor::start(const SceneExecutionRequest& request)
{
    if(state_!=SceneExecutionState::IDLE||requestCount_!=0U)return SceneExecutionResult::ALREADY_RUNNING;
    const SceneExecutionResult requestValidation=validateRequest(request);
    if(requestValidation!=SceneExecutionResult::SUCCESS)return requestValidation;
    const SceneExecutionResult sceneValidation=validateScene(request);
    if(sceneValidation!=SceneExecutionResult::SUCCESS)return sceneValidation;
    const Scene* scene=sceneManager_.findById(request.sceneId);
    if(scene==nullptr||!hasValidCommandIdRange(request,*scene))return SceneExecutionResult::INVALID_COMMAND_ID;
    currentRequest_=request;currentSceneId_=request.sceneId;currentStepArrayIndex_=0U;
    nextCommandId_=request.firstCommandId;delayStartedMs_=0U;delayDurationMs_=0U;
    state_=SceneExecutionState::READY_STEP;lastResult_=SceneExecutionResult::SUCCESS;
    return SceneExecutionResult::SUCCESS;
}

bool SceneExecutor::loadNextRequest()
{
    if(requestCount_==0U)return false;
    const SceneExecutionRequest request=requests_[0];
    for(size_t index=1U;index<requestCount_;++index)requests_[index-1U]=requests_[index];
    --requestCount_;requests_[requestCount_]=SceneExecutionRequest{};
    state_=SceneExecutionState::LOADING;
    const SceneExecutionResult sceneValidation=validateScene(request);
    if(sceneValidation!=SceneExecutionResult::SUCCESS)
    {currentRequest_=request;currentSceneId_=request.sceneId;finishCurrent(SceneExecutionState::FAILED,sceneValidation);return true;}
    const Scene* scene=sceneManager_.findById(request.sceneId);
    if(scene==nullptr||!hasValidCommandIdRange(request,*scene))
    {currentRequest_=request;currentSceneId_=request.sceneId;finishCurrent(SceneExecutionState::FAILED,SceneExecutionResult::INVALID_COMMAND_ID);return true;}
    currentRequest_=request;currentSceneId_=request.sceneId;currentStepArrayIndex_=0U;
    nextCommandId_=request.firstCommandId;delayStartedMs_=0U;delayDurationMs_=0U;
    state_=SceneExecutionState::READY_STEP;lastResult_=SceneExecutionResult::SUCCESS;
    return true;
}

void SceneExecutor::update(uint32_t nowMs)
{
    if(isTerminalSceneExecutionState(state_))
    {clearCurrentRuntime();state_=SceneExecutionState::IDLE;if(requestCount_!=0U)loadNextRequest();return;}
    if(state_==SceneExecutionState::IDLE){loadNextRequest();return;}
    if(state_==SceneExecutionState::WAITING_COMMAND_QUEUE)
    {if(commandQueue_.isFull())return;state_=SceneExecutionState::READY_STEP;}
    if(state_==SceneExecutionState::WAITING_DELAY)
    {
        if(static_cast<uint32_t>(nowMs-delayStartedMs_)<delayDurationMs_)return;
        delayStartedMs_=0U;delayDurationMs_=0U;++currentStepArrayIndex_;
        const Scene* scene=sceneManager_.findById(currentSceneId_);
        if(scene==nullptr){finishCurrent(SceneExecutionState::FAILED,SceneExecutionResult::SCENE_NOT_FOUND);return;}
        if(!scene->isValid()){finishCurrent(SceneExecutionState::FAILED,SceneExecutionResult::SCENE_INVALID);return;}
        if(!scene->enabled){finishCurrent(SceneExecutionState::FAILED,SceneExecutionResult::SCENE_DISABLED);return;}
        if(currentStepArrayIndex_>=scene->stepCount){finishCurrent(SceneExecutionState::COMPLETED,SceneExecutionResult::SUCCESS);return;}
        state_=SceneExecutionState::READY_STEP;
    }
    if(state_==SceneExecutionState::READY_STEP)processReadyStep(nowMs);
}

SceneExecutionResult SceneExecutor::processReadyStep(uint32_t nowMs)
{
    // Scene در هر update دوباره از Manager خوانده می‌شود. تغییر ترتیب Stepها هنگام اجرا
    // هنوز Policy نهایی ندارد؛ در آینده باید Snapshot، Revision Check یا Live-Update Policy انتخاب شود.
    const Scene* scene=sceneManager_.findById(currentSceneId_);
    if(scene==nullptr){finishCurrent(SceneExecutionState::FAILED,SceneExecutionResult::SCENE_NOT_FOUND);return lastResult_;}
    if(!scene->isValid()){finishCurrent(SceneExecutionState::FAILED,SceneExecutionResult::SCENE_INVALID);return lastResult_;}
    if(!scene->enabled){finishCurrent(SceneExecutionState::FAILED,SceneExecutionResult::SCENE_DISABLED);return lastResult_;}
    const SceneStep* step=scene->getStepAt(currentStepArrayIndex_);
    if(step==nullptr||!step->isValid()){finishCurrent(SceneExecutionState::FAILED,SceneExecutionResult::INVALID_STEP);return lastResult_;}
    const bool hasNext=currentStepArrayIndex_+1U<scene->stepCount;
    if(hasNext&&nextCommandId_==UINT32_MAX)
    {finishCurrent(SceneExecutionState::FAILED,SceneExecutionResult::INVALID_COMMAND_ID);return lastResult_;}
    if(commandQueue_.isFull()){state_=SceneExecutionState::WAITING_COMMAND_QUEUE;lastResult_=SceneExecutionResult::COMMAND_QUEUE_FULL;return lastResult_;}

    Command runtimeCommand;
    const AutomationCommandFactoryResult factoryResult=commandFactory_.create(step->command,
        currentRequest_.request,nextCommandId_,nowMs,runtimeCommand);
    if(factoryResult!=AutomationCommandFactoryResult::SUCCESS)
    {finishCurrent(SceneExecutionState::FAILED,SceneExecutionResult::COMMAND_FACTORY_FAILED);return lastResult_;}
    const SceneExecutionResult queueResult=commandQueue_.enqueue(runtimeCommand);
    if(queueResult==SceneExecutionResult::COMMAND_QUEUE_FULL)
    {state_=SceneExecutionState::WAITING_COMMAND_QUEUE;lastResult_=queueResult;return queueResult;}
    if(queueResult!=SceneExecutionResult::SUCCESS)
    {finishCurrent(SceneExecutionState::FAILED,SceneExecutionResult::COMMAND_FACTORY_FAILED);return lastResult_;}

    if(hasNext)++nextCommandId_;
    if(step->delayAfterMs>0U)
    {delayStartedMs_=nowMs;delayDurationMs_=step->delayAfterMs;state_=SceneExecutionState::WAITING_DELAY;lastResult_=SceneExecutionResult::SUCCESS;return lastResult_;}
    ++currentStepArrayIndex_;
    if(currentStepArrayIndex_>=scene->stepCount)finishCurrent(SceneExecutionState::COMPLETED,SceneExecutionResult::SUCCESS);
    else{state_=SceneExecutionState::READY_STEP;lastResult_=SceneExecutionResult::SUCCESS;}
    return lastResult_;
}

SceneExecutionResult SceneExecutor::cancelCurrent()
{
    if(!isRunning())return SceneExecutionResult::NOT_RUNNING;
    delayStartedMs_=0U;delayDurationMs_=0U;
    finishCurrent(SceneExecutionState::CANCELLED,SceneExecutionResult::CANCELLED);
    return SceneExecutionResult::CANCELLED;
}

void SceneExecutor::clearRequests()
{for(size_t i=0;i<SCENE_REQUEST_QUEUE_CAPACITY;++i)requests_[i]=SceneExecutionRequest{};requestCount_=0U;}
void SceneExecutor::clearCurrentRuntime()
{currentRequest_=SceneExecutionRequest{};currentSceneId_=INVALID_SCENE_ID;currentStepArrayIndex_=0U;nextCommandId_=INVALID_COMMAND_ID;delayStartedMs_=0U;delayDurationMs_=0U;}
void SceneExecutor::reset(){clearRequests();clearCurrentRuntime();state_=SceneExecutionState::IDLE;lastResult_=SceneExecutionResult::SUCCESS;}
void SceneExecutor::finishCurrent(SceneExecutionState terminalState,SceneExecutionResult result){state_=terminalState;lastResult_=result;}
SceneExecutionState SceneExecutor::getState()const{return state_;}
bool SceneExecutor::isRunning()const{return state_==SceneExecutionState::LOADING||state_==SceneExecutionState::READY_STEP||state_==SceneExecutionState::WAITING_COMMAND_QUEUE||state_==SceneExecutionState::WAITING_DELAY;}
bool SceneExecutor::hasPendingRequests()const{return requestCount_!=0U;}
SceneId SceneExecutor::getCurrentSceneId()const{return currentSceneId_;}
AutomationStepIndex SceneExecutor::getCurrentStepIndex()const
{const Scene* scene=sceneManager_.findById(currentSceneId_);const SceneStep* step=scene?scene->getStepAt(currentStepArrayIndex_):nullptr;return step?step->stepIndex:INVALID_AUTOMATION_STEP_INDEX;}
size_t SceneExecutor::pendingRequestCount()const{return requestCount_;}size_t SceneExecutor::requestCapacity()const{return SCENE_REQUEST_QUEUE_CAPACITY;}
SceneExecutionResult SceneExecutor::getLastResult()const{return lastResult_;}
