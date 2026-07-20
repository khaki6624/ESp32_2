#ifndef MAIN_CONTROLLER_H
#define MAIN_CONTROLLER_H
#include <MainControllerDependencies.h>
#include <MainControllerRuntimeState.h>
class MainControllerTestAccess;
class MainController
{public:explicit MainController(const MainControllerDependencies&);MainControllerResult begin();MainControllerResult update(MainControllerTimestamp now);bool isInitialized()const;bool isUpdating()const;bool isDegraded()const;MainControllerResult lastResult()const;const MainControllerRuntimeState* state(MainControllerComponent)const;size_t componentCount()const;uint32_t cycleCount()const;
private:friend class MainControllerTestAccess;const MainControllerDependencies& dependencies_;MainControllerRuntimeState states_[MAIN_CONTROLLER_COMPONENT_COUNT];size_t componentCount_;uint32_t cycleCount_;MainControllerResult lastResult_;bool initialized_,updating_,degraded_;void initializeStates();void recordBegin(MainControllerComponent,MainControllerResult);void recordUpdate(MainControllerComponent,MainControllerResult,MainControllerTimestamp);static MainControllerResult combine(MainControllerResult,MainControllerResult);};
#endif
