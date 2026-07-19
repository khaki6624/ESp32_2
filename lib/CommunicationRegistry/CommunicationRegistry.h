#ifndef COMMUNICATION_REGISTRY_H
#define COMMUNICATION_REGISTRY_H
#include <CommunicationBackend.h>
class CommunicationRegistry
{
public:
 CommunicationRegistry();
 CommunicationResult registerBackend(CommunicationBackend& backend);
 CommunicationBackend* find(CommunicationBackendId id)const;
 CommunicationBackend* at(size_t index)const;
 size_t size()const; size_t capacity()const; bool isLocked()const;
 CommunicationResult lock();
private:
 CommunicationBackend* backends_[COMMUNICATION_MAX_BACKENDS]; size_t size_; bool locked_;
};
#endif
