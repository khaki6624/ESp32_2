#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H
#include <Scene.h>
class SceneManager
{
public:
    SceneManager(); void clear();
    AutomationModelResult add(const Scene& scene); AutomationModelResult update(const Scene& scene); AutomationModelResult remove(SceneId id);
    const Scene* findById(SceneId id) const; const Scene* findByName(const char* name) const; const Scene* getAt(size_t index) const;
    bool contains(SceneId id) const; size_t size() const; size_t capacity() const; bool isFull() const; bool isEmpty() const;
private:
    Scene scenes_[SCENE_MANAGER_CAPACITY]; size_t count_;
    size_t findIndexById(SceneId id) const; bool nameBelongsToAnother(const char* name,SceneId id) const;
};
#endif
