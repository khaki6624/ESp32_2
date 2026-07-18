#include <RuntimeAuthorizationPolicy.h>

namespace
{
bool resolvePermission(const Command& command,Permission& output)
{
    switch(command.domain)
    {
        case CommandDomain::CFG:output=Permission::MANAGE_CONFIG;return true;
        case CommandDomain::STORE:output=Permission::MANAGE_STORAGE;return true;
        case CommandDomain::USER:output=Permission::MANAGE_USERS;return true;
        case CommandDomain::SYS:output=Permission::MANAGE_SYSTEM;return true;
        case CommandDomain::NODE:output=Permission::MANAGE_NODES;return true;
        case CommandDomain::SCN:output=Permission::EXECUTE_SCENES;return true;
        case CommandDomain::RULE:output=Permission::EXECUTE_RULES;return true;
        case CommandDomain::SCH:output=Permission::MANAGE_SCHEDULES;return true;
        case CommandDomain::SMS:output=Permission::USE_SMS;return true;
        case CommandDomain::CALL:output=Permission::USE_CALL;return true;
        default:break;
    }
    if(command.isQuery()){output=Permission::QUERY;return true;}
    switch(command.context.risk)
    {
        case CommandRisk::SAFE:output=Permission::QUERY;return true;
        case CommandRisk::ACTION:output=Permission::EXECUTE_ACTION;return true;
        case CommandRisk::SENSITIVE:output=Permission::EXECUTE_SENSITIVE;return true;
        case CommandRisk::DANGEROUS:output=Permission::EXECUTE_DANGEROUS;return true;
        default:return false;
    }
}
}

RuntimeAuthorizationPolicy::RuntimeAuthorizationPolicy(const AuthorizationSubjectProvider& subjectProvider,const UserRegistry& userRegistry,const RoleRegistry& roleRegistry):subjectProvider_(subjectProvider),userRegistry_(userRegistry),roleRegistry_(roleRegistry){}

AuthorizationCheckResult RuntimeAuthorizationPolicy::check(const Command& command,uint32_t nowMs)const
{
    // برای تفکیک Risk نامعتبر از سایر خطاهای ساختاری، فقط Copy محلی اعتبارسنجی می‌شود.
    Command structural=command;structural.context.risk=CommandRisk::SAFE;
    if(!structural.isValid())return AuthorizationCheckResult::INVALID_REQUEST;
    if(!isValidCommandRisk(command.context.risk))return AuthorizationCheckResult::POLICY_ERROR;

    AuthorizationSubject subject;
    const AuthorizationCheckResult resolved=subjectProvider_.resolve(command,nowMs,subject);
    if(!isValidAuthorizationCheckResult(resolved))return AuthorizationCheckResult::POLICY_ERROR;
    if(resolved!=AuthorizationCheckResult::ALLOWED)return resolved;
    if(!subject.isValid())return AuthorizationCheckResult::INVALID_REQUEST;
    if(!subject.authenticated)return AuthorizationCheckResult::UNAUTHORIZED;
    if(subject.isSystem())return AuthorizationCheckResult::ALLOWED;

    const User* user=userRegistry_.find(subject.userId);
    if(user==nullptr)return AuthorizationCheckResult::USER_NOT_FOUND;
    if(!user->enabled)return AuthorizationCheckResult::USER_DISABLED;
    const Role* role=roleRegistry_.find(user->roleId);
    if(role==nullptr||!role->enabled)return AuthorizationCheckResult::PERMISSION_DENIED;
    Permission required;
    if(!resolvePermission(command,required))return AuthorizationCheckResult::POLICY_ERROR;
    return role->hasPermission(required)?AuthorizationCheckResult::ALLOWED:AuthorizationCheckResult::PERMISSION_DENIED;
}
