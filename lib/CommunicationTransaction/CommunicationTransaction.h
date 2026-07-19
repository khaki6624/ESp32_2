#ifndef COMMUNICATION_TRANSACTION_H
#define COMMUNICATION_TRANSACTION_H
#include <CommunicationCommon.h>
class CommunicationRuntime;
class CommunicationTransaction
{
public:
 CommunicationTransaction(); bool isValid()const;
 CommunicationMessageId messageId()const; CommunicationBackendId backendId()const;
 CommunicationType communicationType()const; CommunicationTransactionState state()const;
 CommunicationResult result()const; size_t transferredLength()const;
private:
 friend class CommunicationRuntime;
 CommunicationMessageId messageId_;CommunicationBackendId backendId_;CommunicationType type_;
 CommunicationTransactionState state_;CommunicationResult result_;size_t transferredLength_;bool valid_;
};
#endif
