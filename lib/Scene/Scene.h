#ifndef SCENE_H
#define SCENE_H

#include <AutomationModelCommon.h>
#include <SceneStep.h>

struct Scene
{
    SceneId id;
    char name[AUTOMATION_NAME_MAX_LENGTH];
    SceneStep steps[SCENE_MAX_STEPS];
    uint8_t stepCount;
    bool enabled;

    Scene() : id(INVALID_SCENE_ID), name{}, steps{}, stepCount(0), enabled(false) {}

    bool isValid() const
    {
        if (id == INVALID_SCENE_ID || name[0] == '\0' ||
            !AutomationText::isCanonical(name, sizeof(name)) || stepCount > SCENE_MAX_STEPS) return false;
        for (size_t i = 0; i < stepCount; ++i)
        {
            if (!steps[i].isValid()) return false;
            for (size_t j = i + 1U; j < stepCount; ++j)
                if (steps[i].stepIndex == steps[j].stepIndex) return false;
        }
        return true;
    }
    bool setName(const char* value)
    { return value != nullptr && value[0] != '\0' && AutomationText::set(name, sizeof(name), value); }
    AutomationModelResult addStep(const SceneStep& step)
    {
        if (!step.isValid()) return AutomationModelResult::INVALID_STEP;
        if (isFull()) return AutomationModelResult::STEP_CAPACITY_FULL;
        if (findStep(step.stepIndex) != nullptr) return AutomationModelResult::DUPLICATE_STEP_INDEX;
        // ترتیب اجرای آینده همان ترتیب فیزیکی افزودن است و آرایه Sort نمی‌شود.
        steps[stepCount++] = step;
        return AutomationModelResult::SUCCESS;
    }
    AutomationModelResult updateStep(const SceneStep& step)
    {
        if (!step.isValid()) return AutomationModelResult::INVALID_STEP;
        SceneStep* current = findStepMutable(step.stepIndex);
        if (current == nullptr) return AutomationModelResult::NOT_FOUND;
        *current = step;
        return AutomationModelResult::SUCCESS;
    }
    AutomationModelResult removeStep(AutomationStepIndex stepIndex)
    {
        if (stepIndex == INVALID_AUTOMATION_STEP_INDEX) return AutomationModelResult::INVALID_INDEX;
        size_t index = 0U;
        while (index < stepCount && steps[index].stepIndex != stepIndex) ++index;
        if (index == stepCount) return AutomationModelResult::NOT_FOUND;
        for (size_t i = index + 1U; i < stepCount; ++i) steps[i - 1U] = steps[i];
        --stepCount; steps[stepCount] = SceneStep{};
        return AutomationModelResult::SUCCESS;
    }
    const SceneStep* findStep(AutomationStepIndex stepIndex) const
    { for (size_t i=0;i<stepCount;++i) if(steps[i].stepIndex==stepIndex)return &steps[i]; return nullptr; }
    const SceneStep* getStepAt(size_t index) const { return index < stepCount ? &steps[index] : nullptr; }
    void clearSteps() { for(size_t i=0;i<SCENE_MAX_STEPS;++i)steps[i]=SceneStep{};stepCount=0; }
    void clear() { *this = Scene{}; }
    bool isEmpty() const { return stepCount == 0U; }
    bool isFull() const { return stepCount >= SCENE_MAX_STEPS; }

private:
    SceneStep* findStepMutable(AutomationStepIndex stepIndex)
    { for(size_t i=0;i<stepCount;++i)if(steps[i].stepIndex==stepIndex)return &steps[i];return nullptr; }
};

#endif
