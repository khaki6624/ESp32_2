#ifndef ROLE_REGISTRY_H
#define ROLE_REGISTRY_H
#include <stddef.h>
#include <stdint.h>
#include <Role.h>
constexpr size_t ROLE_REGISTRY_CAPACITY=8U;
enum class RoleRegistryResult:uint8_t{SUCCESS=0,INVALID_ROLE,ROLE_ALREADY_EXISTS,ROLE_NOT_FOUND,REGISTRY_FULL};
inline bool isValidRoleRegistryResult(RoleRegistryResult value){return static_cast<uint8_t>(value)<=static_cast<uint8_t>(RoleRegistryResult::REGISTRY_FULL);}
class RoleRegistry
{
public:
    RoleRegistry();void clear();RoleRegistryResult add(const Role& role);RoleRegistryResult update(const Role& role);RoleRegistryResult remove(RoleId roleId);const Role* find(RoleId roleId)const;size_t size()const;size_t capacity()const;
private:Role roles_[ROLE_REGISTRY_CAPACITY];size_t count_;
};
#endif
