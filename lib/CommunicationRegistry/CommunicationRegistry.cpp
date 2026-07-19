#include "CommunicationRegistry.h"
CommunicationRegistry::CommunicationRegistry():backends_{},size_(0U),locked_(false){}
CommunicationResult CommunicationRegistry::registerBackend(CommunicationBackend& backend)
{
 if(locked_)return CommunicationResult::REGISTRY_LOCKED;
 if(backend.backendId()==0U)return CommunicationResult::INVALID_BACKEND_ID;
 if(!isValidCommunicationType(backend.communicationType()))return CommunicationResult::INVALID_ARGUMENT;
 for(size_t i=0U;i<size_;++i)if(backends_[i]->backendId()==backend.backendId())return CommunicationResult::DUPLICATE_BACKEND_ID;
 if(size_==COMMUNICATION_MAX_BACKENDS)return CommunicationResult::REGISTRY_FULL;
 backends_[size_++]=&backend;return CommunicationResult::SUCCESS;
}
CommunicationBackend* CommunicationRegistry::find(CommunicationBackendId id)const
{if(id==0U)return nullptr;for(size_t i=0U;i<size_;++i)if(backends_[i]->backendId()==id)return backends_[i];return nullptr;}
CommunicationBackend* CommunicationRegistry::at(size_t index)const{return index<size_?backends_[index]:nullptr;}
size_t CommunicationRegistry::size()const{return size_;} size_t CommunicationRegistry::capacity()const{return COMMUNICATION_MAX_BACKENDS;}
bool CommunicationRegistry::isLocked()const{return locked_;} CommunicationResult CommunicationRegistry::lock(){locked_=true;return CommunicationResult::SUCCESS;}
