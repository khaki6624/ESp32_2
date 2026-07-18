#ifndef USER_H
#define USER_H
#include <AuthorizationIdentityCommon.h>
struct User{UserId id;RoleId roleId;bool enabled;User():id(INVALID_USER_ID),roleId(INVALID_ROLE_ID),enabled(false){}bool isValid()const{return id!=INVALID_USER_ID&&roleId!=INVALID_ROLE_ID;}};
#endif
