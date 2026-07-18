#ifndef AUTHORIZATION_POLICY_H
#define AUTHORIZATION_POLICY_H
#include <Command.h>
#include <SecurityCommon.h>
class AuthorizationPolicy{public:virtual ~AuthorizationPolicy()=default;virtual AuthorizationCheckResult check(const Command& command,uint32_t nowMs)const=0;};
#endif
