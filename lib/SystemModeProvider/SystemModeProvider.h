#ifndef SYSTEM_MODE_PROVIDER_H
#define SYSTEM_MODE_PROVIDER_H
#include <Command.h>
#include <SecurityCommon.h>
class SystemModeProvider{public:virtual ~SystemModeProvider()=default;virtual SystemMode getMode()const=0;virtual SystemModeCheckResult check(const Command& command,uint32_t nowMs)const=0;};
#endif
