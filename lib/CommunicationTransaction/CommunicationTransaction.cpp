#include "CommunicationTransaction.h"
CommunicationTransaction::CommunicationTransaction():messageId_(0U),backendId_(0U),type_(CommunicationType::COUNT),state_(CommunicationTransactionState::IDLE),result_(CommunicationResult::INVALID_ARGUMENT),transferredLength_(0U),valid_(false){}
bool CommunicationTransaction::isValid()const
{
 if(!valid_||messageId_==0U||backendId_==0U||!isValidCommunicationType(type_))return false;
 if(state_==CommunicationTransactionState::PENDING)return result_==CommunicationResult::ACCEPTED;
 if(state_==CommunicationTransactionState::RUNNING)return result_==CommunicationResult::IN_PROGRESS||result_==CommunicationResult::BACKEND_RETRY_LATER;
 if(state_==CommunicationTransactionState::SUCCEEDED)return result_==CommunicationResult::SUCCESS;
 if(state_==CommunicationTransactionState::CANCELLED)return result_==CommunicationResult::CANCELLED;
 return state_==CommunicationTransactionState::FAILED&&result_!=CommunicationResult::SUCCESS&&result_!=CommunicationResult::ACCEPTED&&result_!=CommunicationResult::IN_PROGRESS&&result_!=CommunicationResult::CANCELLED;
}
CommunicationMessageId CommunicationTransaction::messageId()const{return messageId_;} CommunicationBackendId CommunicationTransaction::backendId()const{return backendId_;}
CommunicationType CommunicationTransaction::communicationType()const{return type_;} CommunicationTransactionState CommunicationTransaction::state()const{return state_;}
CommunicationResult CommunicationTransaction::result()const{return result_;} size_t CommunicationTransaction::transferredLength()const{return transferredLength_;}
