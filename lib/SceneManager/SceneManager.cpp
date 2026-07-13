#include "SceneManager.h"
#include <string.h>
namespace { constexpr size_t INVALID_INDEX_VALUE=SCENE_MANAGER_CAPACITY; }
SceneManager::SceneManager():scenes_{},count_(0){}
void SceneManager::clear(){for(size_t i=0;i<SCENE_MANAGER_CAPACITY;++i)scenes_[i]=Scene{};count_=0;}
AutomationModelResult SceneManager::add(const Scene& v){if(!v.isValid())return AutomationModelResult::INVALID_MODEL;if(isFull())return AutomationModelResult::CAPACITY_FULL;if(contains(v.id))return AutomationModelResult::DUPLICATE_ID;if(findByName(v.name))return AutomationModelResult::DUPLICATE_NAME;scenes_[count_++]=v;return AutomationModelResult::SUCCESS;}
AutomationModelResult SceneManager::update(const Scene& v){if(!v.isValid())return AutomationModelResult::INVALID_MODEL;size_t i=findIndexById(v.id);if(i==INVALID_INDEX_VALUE)return AutomationModelResult::NOT_FOUND;if(nameBelongsToAnother(v.name,v.id))return AutomationModelResult::DUPLICATE_NAME;scenes_[i]=v;return AutomationModelResult::SUCCESS;}
AutomationModelResult SceneManager::remove(SceneId id){if(id==INVALID_SCENE_ID)return AutomationModelResult::INVALID_ID;size_t i=findIndexById(id);if(i==INVALID_INDEX_VALUE)return AutomationModelResult::NOT_FOUND;for(size_t j=i+1U;j<count_;++j)scenes_[j-1U]=scenes_[j];--count_;scenes_[count_]=Scene{};return AutomationModelResult::SUCCESS;}
size_t SceneManager::findIndexById(SceneId id)const{if(id==INVALID_SCENE_ID)return INVALID_INDEX_VALUE;for(size_t i=0;i<count_;++i)if(scenes_[i].id==id)return i;return INVALID_INDEX_VALUE;}
const Scene* SceneManager::findById(SceneId id)const{size_t i=findIndexById(id);return i==INVALID_INDEX_VALUE?nullptr:&scenes_[i];}
const Scene* SceneManager::findByName(const char* name)const{if(!name||name[0]=='\0')return nullptr;for(size_t i=0;i<count_;++i)if(strcmp(scenes_[i].name,name)==0)return &scenes_[i];return nullptr;}
const Scene* SceneManager::getAt(size_t i)const{return i<count_?&scenes_[i]:nullptr;} bool SceneManager::contains(SceneId id)const{return findIndexById(id)!=INVALID_INDEX_VALUE;}
bool SceneManager::nameBelongsToAnother(const char* n,SceneId id)const{const Scene* p=findByName(n);return p&&p->id!=id;}
size_t SceneManager::size()const{return count_;}size_t SceneManager::capacity()const{return SCENE_MANAGER_CAPACITY;}bool SceneManager::isFull()const{return count_>=SCENE_MANAGER_CAPACITY;}bool SceneManager::isEmpty()const{return count_==0U;}
