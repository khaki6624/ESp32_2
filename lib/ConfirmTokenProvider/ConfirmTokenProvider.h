#ifndef CONFIRM_TOKEN_PROVIDER_H
#define CONFIRM_TOKEN_PROVIDER_H
#include <Command.h>
#include <SecurityCommon.h>
class ConfirmTokenProvider{public:virtual ~ConfirmTokenProvider()=default;virtual ConfirmTokenCheckResult check(const Command& command,uint32_t nowMs)const=0;virtual ConfirmTokenCheckResult consume(const Command& command,uint32_t nowMs)=0;};
#endif
