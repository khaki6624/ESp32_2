#ifndef MAIN_CONTROLLER_RUNTIME_STATE_H
#define MAIN_CONTROLLER_RUNTIME_STATE_H
#include <MainControllerCommon.h>
class MainController;
class MainControllerTestAccess;
class MainControllerRuntimeState
{public:MainControllerRuntimeState();bool isValid()const;MainControllerComponent component()const;bool initialized()const;MainControllerResult lastResult()const;uint32_t updateCount()const;uint32_t failureCount()const;uint32_t consecutiveFailureCount()const;MainControllerTimestamp lastUpdateAt()const;MainControllerTimestamp lastSuccessAt()const;
private:friend class MainController;friend class MainControllerTestAccess;MainControllerComponent component_;MainControllerResult lastResult_;uint32_t updateCount_,failureCount_,consecutiveFailureCount_;MainControllerTimestamp lastUpdateAt_,lastSuccessAt_;bool initialized_,valid_;};
#endif
