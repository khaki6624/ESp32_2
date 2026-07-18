#ifndef AUTHORIZATION_SUBJECT_PROVIDER_H
#define AUTHORIZATION_SUBJECT_PROVIDER_H
#include <AuthorizationIdentityCommon.h>
#include <Command.h>
#include <SecurityCommon.h>
class AuthorizationSubjectProvider{public:virtual ~AuthorizationSubjectProvider()=default;virtual AuthorizationCheckResult resolve(const Command& command,uint32_t nowMs,AuthorizationSubject& output)const=0;};
#endif
