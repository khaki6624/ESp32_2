#include "StorageTransaction.h"

StorageTransaction::StorageTransaction() : operationId_(INVALID_STORAGE_OPERATION_ID),
    type_(StorageOperationType::COUNT), state_(StorageOperationState::IDLE),
    result_(StorageResult::INVALID_OPERATION), transferredLength_(0U), valid_(false) {}

bool StorageTransaction::isValid() const
{
    if (!valid_ || operationId_ == INVALID_STORAGE_OPERATION_ID ||
        !isValidStorageOperationType(type_) ||
        state_ == StorageOperationState::IDLE || state_ == StorageOperationState::COUNT)
        return false;
    if (state_ == StorageOperationState::SUCCEEDED) return result_ == StorageResult::SUCCESS;
    if (state_ == StorageOperationState::CANCELLED) return result_ == StorageResult::CANCELLED;
    if (state_ == StorageOperationState::PENDING) return result_ == StorageResult::ACCEPTED;
    if (state_ == StorageOperationState::RUNNING)
        return result_ == StorageResult::IN_PROGRESS ||
               result_ == StorageResult::BACKEND_RETRY_LATER;
    return state_ == StorageOperationState::FAILED &&
           result_ != StorageResult::SUCCESS && result_ != StorageResult::ACCEPTED &&
           result_ != StorageResult::IN_PROGRESS && result_ != StorageResult::CANCELLED;
}

StorageOperationId StorageTransaction::operationId() const { return operationId_; }
StorageOperationType StorageTransaction::type() const { return type_; }
StorageOperationState StorageTransaction::state() const { return state_; }
StorageResult StorageTransaction::result() const { return result_; }
size_t StorageTransaction::transferredLength() const { return transferredLength_; }
