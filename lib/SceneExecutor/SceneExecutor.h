#ifndef SCENE_EXECUTOR_H
#define SCENE_EXECUTOR_H

#include <AutomationCommandFactory.h>
#include <SceneExecutionQueue.h>
#include <SceneManager.h>

class SceneExecutor
{
public:
    SceneExecutor(const SceneManager& sceneManager,const AutomationCommandFactory& commandFactory,
                  SceneExecutionQueue& commandQueue);
    SceneExecutionResult enqueue(const SceneExecutionRequest& request);
    SceneExecutionResult start(const SceneExecutionRequest& request);
    void update(uint32_t nowMs);
    SceneExecutionResult cancelCurrent();
    void clearRequests();
    void reset();
    SceneExecutionState getState() const;
    bool isRunning() const;bool hasPendingRequests() const;
    SceneId getCurrentSceneId() const;
    AutomationStepIndex getCurrentStepIndex() const;
    size_t pendingRequestCount() const;size_t requestCapacity() const;
    SceneExecutionResult getLastResult() const;
private:
    const SceneManager& sceneManager_;
    const AutomationCommandFactory& commandFactory_;
    SceneExecutionQueue& commandQueue_;
    SceneExecutionRequest requests_[SCENE_REQUEST_QUEUE_CAPACITY];
    size_t requestCount_;
    SceneExecutionRequest currentRequest_;
    SceneExecutionState state_;
    SceneExecutionResult lastResult_;
    SceneId currentSceneId_;
    size_t currentStepArrayIndex_;
    CommandId nextCommandId_;
    uint32_t delayStartedMs_;
    uint32_t delayDurationMs_;

    bool loadNextRequest();
    void finishCurrent(SceneExecutionState terminalState,SceneExecutionResult result);
    void clearCurrentRuntime();
    SceneExecutionResult processReadyStep(uint32_t nowMs);
    SceneExecutionResult validateRequest(const SceneExecutionRequest& request) const;
    SceneExecutionResult validateScene(const SceneExecutionRequest& request) const;
    bool hasValidCommandIdRange(const SceneExecutionRequest& request,const Scene& scene) const;
};

// COMPLETED فقط تحویل همه Stepها به Queue خروجی را بیان می‌کند و به معنی موفقیت
// سخت‌افزاری Commandها نیست. Commandهای SCN نیز فقط Queue می‌شوند و اجرای بازگشتی،
// Depth، Correlation و Cycle Detection به Batchهای آینده واگذار شده‌اند.

#endif
