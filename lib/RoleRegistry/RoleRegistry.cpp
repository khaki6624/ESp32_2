#include <RoleRegistry.h>
RoleRegistry::RoleRegistry():roles_{},count_(0U){}
void RoleRegistry::clear(){for(size_t i=0U;i<ROLE_REGISTRY_CAPACITY;++i)roles_[i]=Role{};count_=0U;}
RoleRegistryResult RoleRegistry::add(const Role& role){if(!role.isValid())return RoleRegistryResult::INVALID_ROLE;if(find(role.id)!=nullptr)return RoleRegistryResult::ROLE_ALREADY_EXISTS;if(count_==ROLE_REGISTRY_CAPACITY)return RoleRegistryResult::REGISTRY_FULL;roles_[count_++]=role;return RoleRegistryResult::SUCCESS;}
RoleRegistryResult RoleRegistry::update(const Role& role){if(!role.isValid())return RoleRegistryResult::INVALID_ROLE;for(size_t i=0U;i<count_;++i)if(roles_[i].id==role.id){roles_[i]=role;return RoleRegistryResult::SUCCESS;}return RoleRegistryResult::ROLE_NOT_FOUND;}
RoleRegistryResult RoleRegistry::remove(RoleId roleId){if(roleId==INVALID_ROLE_ID)return RoleRegistryResult::ROLE_NOT_FOUND;for(size_t i=0U;i<count_;++i)if(roles_[i].id==roleId){for(size_t j=i+1U;j<count_;++j)roles_[j-1U]=roles_[j];--count_;roles_[count_]=Role{};return RoleRegistryResult::SUCCESS;}return RoleRegistryResult::ROLE_NOT_FOUND;}
const Role* RoleRegistry::find(RoleId roleId)const{if(roleId==INVALID_ROLE_ID)return nullptr;for(size_t i=0U;i<count_;++i)if(roles_[i].id==roleId)return &roles_[i];return nullptr;}
size_t RoleRegistry::size()const{return count_;}size_t RoleRegistry::capacity()const{return ROLE_REGISTRY_CAPACITY;}
