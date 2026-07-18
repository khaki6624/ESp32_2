#include <UserRegistry.h>
UserRegistry::UserRegistry():users_{},count_(0U){}
void UserRegistry::clear(){for(size_t i=0U;i<USER_REGISTRY_CAPACITY;++i)users_[i]=User{};count_=0U;}
UserRegistryResult UserRegistry::add(const User& user){if(!user.isValid())return UserRegistryResult::INVALID_USER;if(find(user.id)!=nullptr)return UserRegistryResult::USER_ALREADY_EXISTS;if(count_==USER_REGISTRY_CAPACITY)return UserRegistryResult::REGISTRY_FULL;users_[count_++]=user;return UserRegistryResult::SUCCESS;}
UserRegistryResult UserRegistry::update(const User& user){if(!user.isValid())return UserRegistryResult::INVALID_USER;for(size_t i=0U;i<count_;++i)if(users_[i].id==user.id){users_[i]=user;return UserRegistryResult::SUCCESS;}return UserRegistryResult::USER_NOT_FOUND;}
UserRegistryResult UserRegistry::remove(UserId userId){if(userId==INVALID_USER_ID)return UserRegistryResult::USER_NOT_FOUND;for(size_t i=0U;i<count_;++i)if(users_[i].id==userId){for(size_t j=i+1U;j<count_;++j)users_[j-1U]=users_[j];--count_;users_[count_]=User{};return UserRegistryResult::SUCCESS;}return UserRegistryResult::USER_NOT_FOUND;}
const User* UserRegistry::find(UserId userId)const{if(userId==INVALID_USER_ID)return nullptr;for(size_t i=0U;i<count_;++i)if(users_[i].id==userId)return &users_[i];return nullptr;}
size_t UserRegistry::size()const{return count_;}size_t UserRegistry::capacity()const{return USER_REGISTRY_CAPACITY;}
