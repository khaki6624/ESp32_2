#ifndef ROLE_H
#define ROLE_H
#include <AuthorizationIdentityCommon.h>
struct Role{RoleId id;bool enabled;PermissionMask permissions;Role():id(INVALID_ROLE_ID),enabled(false),permissions(NO_PERMISSIONS){}bool isValid()const{return id!=INVALID_ROLE_ID&&isValidPermissionMask(permissions);}bool hasPermission(Permission permission)const{return ::hasPermission(permissions,permission);}};
#endif
