#ifndef AUTHORIZATION_IDENTITY_COMMON_H
#define AUTHORIZATION_IDENTITY_COMMON_H

#include <CommandCommon.h>

using RoleId = uint16_t;
using PermissionMask = uint64_t;

constexpr RoleId INVALID_ROLE_ID = 0U;
constexpr PermissionMask NO_PERMISSIONS = 0ULL;

enum class Permission : uint8_t
{
    QUERY = 0, EXECUTE_ACTION, EXECUTE_SENSITIVE, EXECUTE_DANGEROUS,
    MANAGE_CONFIG, MANAGE_STORAGE, MANAGE_USERS, MANAGE_SYSTEM, MANAGE_NODES,
    EXECUTE_SCENES, EXECUTE_RULES, MANAGE_SCHEDULES, USE_SMS, USE_CALL, COUNT
};

inline bool isValidPermission(Permission permission)
{ return static_cast<uint8_t>(permission)<static_cast<uint8_t>(Permission::COUNT); }
inline PermissionMask permissionToMask(Permission permission)
{
    static_assert(static_cast<uint8_t>(Permission::COUNT)<=64U,"Permission count exceeds PermissionMask capacity");
    if(!isValidPermission(permission))return NO_PERMISSIONS;
    return PermissionMask{1ULL}<<static_cast<uint8_t>(permission);
}
inline bool hasPermission(PermissionMask mask,Permission permission)
{ const PermissionMask bit=permissionToMask(permission);return bit!=NO_PERMISSIONS&&(mask&bit)!=0ULL; }
inline bool isValidPermissionMask(PermissionMask mask)
{
    constexpr uint8_t count=static_cast<uint8_t>(Permission::COUNT);
    static_assert(count<=64U,"Permission count exceeds PermissionMask capacity");
    if(count==64U)return true;
    const PermissionMask validMask=(PermissionMask{1ULL}<<count)-PermissionMask{1ULL};
    return (mask&~validMask)==NO_PERMISSIONS;
}

enum class AuthorizationSubjectType : uint8_t { NONE=0, USER, SYSTEM };
inline bool isValidAuthorizationSubjectType(AuthorizationSubjectType type)
{ return type==AuthorizationSubjectType::NONE||type==AuthorizationSubjectType::USER||type==AuthorizationSubjectType::SYSTEM; }

struct AuthorizationSubject
{
    AuthorizationSubjectType type;UserId userId;bool authenticated;
    AuthorizationSubject():type(AuthorizationSubjectType::NONE),userId(INVALID_USER_ID),authenticated(false){}
    bool isValid()const
    {
        if(!isValidAuthorizationSubjectType(type)||type==AuthorizationSubjectType::NONE)return false;
        return type==AuthorizationSubjectType::USER?userId!=INVALID_USER_ID:userId==INVALID_USER_ID;
    }
    bool isUser()const{return type==AuthorizationSubjectType::USER;}
    bool isSystem()const{return type==AuthorizationSubjectType::SYSTEM;}
};

#endif
