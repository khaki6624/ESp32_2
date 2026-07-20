#include "MainController.h"

namespace
{
constexpr MainControllerComponent BEGIN_ORDER[]={
 MainControllerComponent::CONFIGURATION,MainControllerComponent::STORAGE,
 MainControllerComponent::EVENT,MainControllerComponent::EVENT_TO_NOTIFICATION,
 MainControllerComponent::NODE_TO_EVENT,MainControllerComponent::COMMAND_TO_EVENT,
 MainControllerComponent::COMMUNICATION,MainControllerComponent::NODE,
 MainControllerComponent::NOTIFICATION,MainControllerComponent::COMMAND_EXECUTION,
 MainControllerComponent::SCHEDULER,MainControllerComponent::RULE,
 MainControllerComponent::SCENE,MainControllerComponent::LOGGER};
constexpr MainControllerComponent UPDATE_ORDER[]={
 MainControllerComponent::CONFIGURATION,MainControllerComponent::COMMUNICATION,
 MainControllerComponent::COMMAND_EXECUTION,MainControllerComponent::NODE,
 MainControllerComponent::SCHEDULER,MainControllerComponent::RULE,
 MainControllerComponent::SCENE,MainControllerComponent::EVENT,
 MainControllerComponent::NOTIFICATION,MainControllerComponent::LOGGER,
 MainControllerComponent::STORAGE};
uint8_t priority(MainControllerResult r){switch(r){case MainControllerResult::INTERNAL_ERROR:return 7U;case MainControllerResult::COMPONENT_FAILED:return 6U;case MainControllerResult::DEGRADED:return 5U;case MainControllerResult::RETRY_LATER:return 4U;case MainControllerResult::IN_PROGRESS:return 3U;case MainControllerResult::SUCCESS:return 2U;case MainControllerResult::NO_CHANGE:return 1U;default:return 7U;}}
bool failure(MainControllerResult r){return r==MainControllerResult::DEGRADED||r==MainControllerResult::COMPONENT_FAILED||r==MainControllerResult::INTERNAL_ERROR;}
bool fatal(MainControllerResult r){return r==MainControllerResult::COMPONENT_FAILED||r==MainControllerResult::INTERNAL_ERROR;}
bool dependsOnConfiguration(MainControllerComponent c){return c==MainControllerComponent::COMMAND_EXECUTION||c==MainControllerComponent::SCHEDULER||c==MainControllerComponent::RULE||c==MainControllerComponent::SCENE;}
bool dependsOnEvent(MainControllerComponent c){return c==MainControllerComponent::EVENT_TO_NOTIFICATION||c==MainControllerComponent::NODE_TO_EVENT||c==MainControllerComponent::COMMAND_TO_EVENT;}
// Event failure does not block any active update-stage component.
// Notification, Logger and Storage must continue draining prior work.
// Event integration adapters are passive and are not polled here.
bool shouldSkipUpdate(MainControllerComponent c,bool configurationFatal){return configurationFatal&&dependsOnConfiguration(c);}
MainControllerResult normalizeBegin(MainControllerResult r){if(!isValidMainControllerResult(r))return MainControllerResult::INTERNAL_ERROR;if(r==MainControllerResult::INITIALIZATION_FAILED)return MainControllerResult::COMPONENT_FAILED;if(r==MainControllerResult::NOT_INITIALIZED||r==MainControllerResult::ALREADY_UPDATING||r==MainControllerResult::INVALID_ARGUMENT)return MainControllerResult::INTERNAL_ERROR;return r;}
MainControllerResult normalizeUpdate(MainControllerResult r){if(!isValidMainControllerResult(r)||r==MainControllerResult::NOT_INITIALIZED||r==MainControllerResult::ALREADY_UPDATING||r==MainControllerResult::INVALID_ARGUMENT||r==MainControllerResult::INITIALIZATION_FAILED)return MainControllerResult::INTERNAL_ERROR;return r;}
}

MainController::MainController(const MainControllerDependencies& d):dependencies_(d),states_{},componentCount_(0U),cycleCount_(0U),lastResult_(MainControllerResult::NOT_INITIALIZED),initialized_(false),beginning_(false),updating_(false),degraded_(false){initializeStates();}
void MainController::initializeStates(){componentCount_=0U;for(size_t i=0U;i<MAIN_CONTROLLER_COMPONENT_COUNT;++i){states_[i]=MainControllerRuntimeState{};states_[i].component_=static_cast<MainControllerComponent>(i);states_[i].valid_=true;MainControllerComponentPort* p=dependencies_.components[i];if(p!=nullptr&&p->component()==static_cast<MainControllerComponent>(i))++componentCount_;}}
void MainController::recordBegin(MainControllerComponent c,MainControllerResult r){MainControllerRuntimeState& s=states_[static_cast<size_t>(c)];s.lastResult_=r;s.initialized_=r==MainControllerResult::SUCCESS||r==MainControllerResult::NO_CHANGE||r==MainControllerResult::DEGRADED;}
MainControllerResult MainController::begin(MainControllerTimestamp now)
{
 if(beginning_||updating_)return MainControllerResult::ALREADY_UPDATING;
 beginning_=true;
 degraded_=false;
 if(dependencies_.requiredCount()==0U){initialized_=false;lastResult_=MainControllerResult::INITIALIZATION_FAILED;beginning_=false;return lastResult_;}
 if(componentCount_==0U){for(size_t i=0U;i<MAIN_CONTROLLER_COMPONENT_COUNT;++i)if(dependencies_.isRequired(static_cast<MainControllerComponent>(i)))recordBegin(static_cast<MainControllerComponent>(i),MainControllerResult::INITIALIZATION_FAILED);initialized_=false;degraded_=false;lastResult_=MainControllerResult::INITIALIZATION_FAILED;beginning_=false;return lastResult_;}
 bool criticalFailed=false,configurationFailed=false,eventFailed=false;MainControllerResult overall=MainControllerResult::SUCCESS;
 for(size_t i=0U;i<sizeof(BEGIN_ORDER)/sizeof(BEGIN_ORDER[0]);++i)
 {
  const MainControllerComponent c=BEGIN_ORDER[i];MainControllerComponentPort* p=dependencies_.get(c);
  if(p==nullptr){if(dependencies_.isRequired(c)){recordBegin(c,MainControllerResult::INITIALIZATION_FAILED);criticalFailed=true;if(c==MainControllerComponent::CONFIGURATION)configurationFailed=true;if(c==MainControllerComponent::EVENT)eventFailed=true;}continue;}
  if((configurationFailed&&dependsOnConfiguration(c))||(eventFailed&&dependsOnEvent(c))){recordBegin(c,MainControllerResult::INITIALIZATION_FAILED);continue;}
  if(p->component()!=c){recordBegin(c,MainControllerResult::INTERNAL_ERROR);criticalFailed=true;overall=MainControllerResult::INTERNAL_ERROR;continue;}
  const MainControllerResult r=normalizeBegin(p->hasBegin()?p->begin(now):MainControllerResult::SUCCESS);recordBegin(c,r);
  if(r==MainControllerResult::INTERNAL_ERROR){criticalFailed=true;if(c==MainControllerComponent::CONFIGURATION)configurationFailed=true;if(c==MainControllerComponent::EVENT)eventFailed=true;}
  else if(r==MainControllerResult::COMPONENT_FAILED){if(p->critical()||dependencies_.isRequired(c)){criticalFailed=true;if(c==MainControllerComponent::CONFIGURATION)configurationFailed=true;if(c==MainControllerComponent::EVENT)eventFailed=true;}else overall=MainControllerResult::DEGRADED;}
  else if(p->critical()&&r!=MainControllerResult::SUCCESS&&r!=MainControllerResult::NO_CHANGE&&r!=MainControllerResult::DEGRADED)criticalFailed=true;
  overall=combine(overall,r);
 }
 if(criticalFailed){initialized_=false;lastResult_=MainControllerResult::INITIALIZATION_FAILED;beginning_=false;return lastResult_;}
 initialized_=true;degraded_=overall==MainControllerResult::DEGRADED||overall==MainControllerResult::COMPONENT_FAILED;lastResult_=degraded_?MainControllerResult::DEGRADED:MainControllerResult::SUCCESS;beginning_=false;return lastResult_;
}
MainControllerResult MainController::combine(MainControllerResult a,MainControllerResult b){if(!isValidMainControllerResult(a)||!isValidMainControllerResult(b))return MainControllerResult::INTERNAL_ERROR;return priority(b)>priority(a)?b:a;}
void MainController::recordUpdate(MainControllerComponent c,MainControllerResult r,MainControllerTimestamp now){MainControllerRuntimeState& s=states_[static_cast<size_t>(c)];s.lastResult_=r;mainControllerSaturatingIncrement(s.updateCount_);s.lastUpdateAt_=now;if(r==MainControllerResult::SUCCESS){s.consecutiveFailureCount_=0U;s.lastSuccessAt_=now;}else if(failure(r)){mainControllerSaturatingIncrement(s.failureCount_);mainControllerSaturatingIncrement(s.consecutiveFailureCount_);}}
MainControllerResult MainController::update(MainControllerTimestamp now)
{
 if(!initialized_)return MainControllerResult::NOT_INITIALIZED;
 if(beginning_||updating_)return MainControllerResult::ALREADY_UPDATING;
 updating_=true;
 MainControllerResult overall=MainControllerResult::NO_CHANGE;bool configurationFatal=false;
 for(size_t i=0U;i<sizeof(UPDATE_ORDER)/sizeof(UPDATE_ORDER[0]);++i)
 {
  const MainControllerComponent c=UPDATE_ORDER[i];MainControllerComponentPort* p=dependencies_.get(c);if(p==nullptr||!p->hasUpdate()||shouldSkipUpdate(c,configurationFatal))continue;
  const MainControllerResult r=normalizeUpdate(p->update(now));recordUpdate(c,r,now);overall=combine(overall,r);
  if(fatal(r)&&c==MainControllerComponent::CONFIGURATION)configurationFatal=true;
 }
 mainControllerSaturatingIncrement(cycleCount_);degraded_=overall==MainControllerResult::DEGRADED||overall==MainControllerResult::COMPONENT_FAILED||overall==MainControllerResult::INTERNAL_ERROR;lastResult_=overall;updating_=false;return overall;
}
bool MainController::isInitialized()const{return initialized_;}bool MainController::isUpdating()const{return updating_||beginning_;}bool MainController::isDegraded()const{return degraded_;}MainControllerResult MainController::lastResult()const{return lastResult_;}const MainControllerRuntimeState* MainController::state(MainControllerComponent c)const{return isValidMainControllerComponent(c)?&states_[static_cast<size_t>(c)]:nullptr;}size_t MainController::componentCount()const{return componentCount_;}uint32_t MainController::cycleCount()const{return cycleCount_;}
