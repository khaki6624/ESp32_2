#ifndef SAFETY_POLICY_H
#define SAFETY_POLICY_H
#include <Command.h>
#include <SecurityCommon.h>
class SafetyPolicy{public:virtual ~SafetyPolicy()=default;virtual SafetyCheckResult check(const Command& command,uint32_t nowMs)const=0;};
#endif
