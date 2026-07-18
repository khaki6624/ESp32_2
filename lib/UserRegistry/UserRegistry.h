#ifndef USER_REGISTRY_H
#define USER_REGISTRY_H
#include <stddef.h>
#include <stdint.h>
#include <User.h>
constexpr size_t USER_REGISTRY_CAPACITY=16U;
enum class UserRegistryResult:uint8_t{SUCCESS=0,INVALID_USER,USER_ALREADY_EXISTS,USER_NOT_FOUND,REGISTRY_FULL};
inline bool isValidUserRegistryResult(UserRegistryResult value){return static_cast<uint8_t>(value)<=static_cast<uint8_t>(UserRegistryResult::REGISTRY_FULL);}
class UserRegistry
{
public:
    UserRegistry();void clear();UserRegistryResult add(const User& user);UserRegistryResult update(const User& user);UserRegistryResult remove(UserId userId);const User* find(UserId userId)const;size_t size()const;size_t capacity()const;
private:User users_[USER_REGISTRY_CAPACITY];size_t count_;
};
#endif
