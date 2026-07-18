#ifndef RUNTIME_AUTHORIZATION_POLICY_H
#define RUNTIME_AUTHORIZATION_POLICY_H
#include <AuthorizationPolicy.h>
#include <AuthorizationSubjectProvider.h>
#include <RoleRegistry.h>
#include <UserRegistry.h>
class RuntimeAuthorizationPolicy final:public AuthorizationPolicy
{
public:
    RuntimeAuthorizationPolicy(const AuthorizationSubjectProvider& subjectProvider,const UserRegistry& userRegistry,const RoleRegistry& roleRegistry);
    AuthorizationCheckResult check(const Command& command,uint32_t nowMs)const override;
private:
    const AuthorizationSubjectProvider& subjectProvider_;const UserRegistry& userRegistry_;const RoleRegistry& roleRegistry_;
};
#endif
