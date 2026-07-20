#ifndef MAIN_CONTROLLER_DEPENDENCIES_H
#define MAIN_CONTROLLER_DEPENDENCIES_H
#include <MainControllerCommon.h>

// Limited orchestration seam for concrete, non-virtual frozen runtimes. A
// Composition Root owns each small binding and every underlying runtime.
class MainControllerComponentPort
{public:virtual ~MainControllerComponentPort()=default;virtual MainControllerComponent component()const=0;virtual bool critical()const=0;virtual bool hasBegin()const=0;virtual bool hasUpdate()const=0;virtual MainControllerResult begin(MainControllerTimestamp now)=0;virtual MainControllerResult update(MainControllerTimestamp now)=0;};
struct MainControllerDependencies
{MainControllerComponentPort* components[MAIN_CONTROLLER_COMPONENT_COUNT];MainControllerDependencies():components{}{}bool set(MainControllerComponentPort& port){const MainControllerComponent id=port.component();if(!isValidMainControllerComponent(id))return false;const size_t i=static_cast<size_t>(id);if(components[i]!=nullptr)return false;components[i]=&port;return true;}MainControllerComponentPort* get(MainControllerComponent id)const{return isValidMainControllerComponent(id)?components[static_cast<size_t>(id)]:nullptr;}};
#endif
