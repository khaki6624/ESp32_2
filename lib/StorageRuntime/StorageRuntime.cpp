#include "StorageRuntime.h"

StorageRuntime::StorageRuntime(StorageBackend& backend) : backend_(backend),
    activeRequest_{}, transaction_{}, initialized_(false), busy_(false) {}

void StorageRuntime::setTransaction(const StorageRequest& request,
    StorageOperationState state, StorageResult result, size_t transferredLength)
{
    transaction_.operationId_ = request.operationId();
    transaction_.type_ = request.type();
    transaction_.state_ = state;
    transaction_.result_ = result;
    transaction_.transferredLength_ = transferredLength;
    transaction_.valid_ = true;
}

StorageResult StorageRuntime::begin()
{
    if (busy_) return StorageResult::BUSY;
    initialized_ = false;
    activeRequest_ = StorageRequest{};
    transaction_ = StorageTransaction{};
    const StorageBackendResult result = backend_.begin();
    if (!isValidStorageBackendResult(result)) return StorageResult::INTERNAL_ERROR;
    if (result == StorageBackendResult::SUCCESS)
    {
        initialized_ = true;
        return StorageResult::SUCCESS;
    }
    if (result == StorageBackendResult::RETRY_LATER)
        return StorageResult::BACKEND_RETRY_LATER;
    if (result == StorageBackendResult::ACCEPTED ||
        result == StorageBackendResult::IN_PROGRESS)
        return StorageResult::BACKEND_NOT_READY;
    return StorageResult::BACKEND_FAILED;
}

bool StorageRuntime::isInitialized() const { return initialized_; }
bool StorageRuntime::isBusy() const { return busy_; }

StorageResult StorageRuntime::submit(const StorageRequest& request)
{
    if (!initialized_) return StorageResult::NOT_INITIALIZED;
    if (busy_) return StorageResult::BUSY;
    if (!request.isValid()) return StorageResult::INVALID_ARGUMENT;
    if (!backend_.isReady()) return StorageResult::BACKEND_NOT_READY;

    size_t transferredLength = 0U;
    const StorageBackendResult backendResult = backend_.start(request, transferredLength);
    const bool operationAccepted = backendResult == StorageBackendResult::SUCCESS ||
        backendResult == StorageBackendResult::ACCEPTED ||
        backendResult == StorageBackendResult::IN_PROGRESS;
    if (!isTransferredLengthValid(request, transferredLength) ||
        (!operationAccepted && transferredLength != 0U))
    {
        setTransaction(request, StorageOperationState::FAILED,
            StorageResult::INTERNAL_ERROR, transferredLength);
        return StorageResult::INTERNAL_ERROR;
    }
    if (!isValidStorageBackendResult(backendResult))
    {
        setTransaction(request, StorageOperationState::FAILED,
            StorageResult::INTERNAL_ERROR, transferredLength);
        return StorageResult::INTERNAL_ERROR;
    }

    if (backendResult == StorageBackendResult::ACCEPTED)
    {
        activeRequest_ = request;
        setTransaction(request, StorageOperationState::PENDING,
            StorageResult::ACCEPTED, transferredLength);
        busy_ = true;
        return StorageResult::ACCEPTED;
    }
    if (backendResult == StorageBackendResult::IN_PROGRESS)
    {
        activeRequest_ = request;
        setTransaction(request, StorageOperationState::RUNNING,
            StorageResult::IN_PROGRESS, transferredLength);
        busy_ = true;
        return StorageResult::ACCEPTED;
    }
    if (backendResult == StorageBackendResult::SUCCESS)
    {
        activeRequest_ = request;
        setTransaction(request, StorageOperationState::SUCCEEDED,
            StorageResult::SUCCESS, transferredLength);
        return StorageResult::SUCCESS;
    }

    StorageResult result = StorageResult::BACKEND_FAILED;
    StorageOperationState state = StorageOperationState::FAILED;
    if (backendResult == StorageBackendResult::RETRY_LATER)
        result = StorageResult::BACKEND_RETRY_LATER;
    else if (backendResult == StorageBackendResult::NOT_FOUND)
        result = StorageResult::NOT_FOUND;
    else if (backendResult == StorageBackendResult::ALREADY_EXISTS)
        result = StorageResult::ALREADY_EXISTS;
    else if (backendResult == StorageBackendResult::BUFFER_TOO_SMALL)
        result = StorageResult::BUFFER_TOO_SMALL;
    else if (backendResult == StorageBackendResult::CANCELLED)
    {
        result = StorageResult::CANCELLED;
        state = StorageOperationState::CANCELLED;
    }
    setTransaction(request, state, result, transferredLength);
    return result;
}

bool StorageRuntime::isTransferredLengthValid(const StorageRequest& request,
    size_t transferredLength) const
{
    if (!request.isValid()) return false;
    if (request.type() == StorageOperationType::READ)
        return transferredLength <= request.output().capacity();
    if (request.type() == StorageOperationType::WRITE)
        return transferredLength <= request.input().length();
    return transferredLength == 0U;
}

StorageResult StorageRuntime::finishFailure(StorageResult result)
{
    transaction_.state_ = StorageOperationState::FAILED;
    transaction_.result_ = result;
    busy_ = false;
    return result;
}

StorageResult StorageRuntime::update()
{
    if (!initialized_) return StorageResult::NOT_INITIALIZED;
    if (!busy_) return StorageResult::INVALID_OPERATION;

    size_t transferredLength = transaction_.transferredLength_;
    const StorageBackendResult backendResult = backend_.update(transferredLength);
    if (transferredLength < transaction_.transferredLength_ ||
        !isTransferredLengthValid(activeRequest_, transferredLength))
        return finishFailure(StorageResult::INTERNAL_ERROR);
    transaction_.transferredLength_ = transferredLength;
    if (!isValidStorageBackendResult(backendResult))
        return finishFailure(StorageResult::INTERNAL_ERROR);

    if (backendResult == StorageBackendResult::IN_PROGRESS)
    {
        transaction_.state_ = StorageOperationState::RUNNING;
        transaction_.result_ = StorageResult::IN_PROGRESS;
        return StorageResult::IN_PROGRESS;
    }
    if (backendResult == StorageBackendResult::RETRY_LATER)
    {
        transaction_.state_ = StorageOperationState::RUNNING;
        transaction_.result_ = StorageResult::BACKEND_RETRY_LATER;
        return StorageResult::BACKEND_RETRY_LATER;
    }
    if (backendResult == StorageBackendResult::SUCCESS)
    {
        transaction_.state_ = StorageOperationState::SUCCEEDED;
        transaction_.result_ = StorageResult::SUCCESS;
        busy_ = false;
        return StorageResult::SUCCESS;
    }
    if (backendResult == StorageBackendResult::CANCELLED)
    {
        transaction_.state_ = StorageOperationState::CANCELLED;
        transaction_.result_ = StorageResult::CANCELLED;
        busy_ = false;
        return StorageResult::CANCELLED;
    }
    if (backendResult == StorageBackendResult::NOT_FOUND)
        return finishFailure(StorageResult::NOT_FOUND);
    if (backendResult == StorageBackendResult::BUFFER_TOO_SMALL)
        return finishFailure(StorageResult::BUFFER_TOO_SMALL);
    if (backendResult == StorageBackendResult::FAILED)
        return finishFailure(StorageResult::BACKEND_FAILED);
    return finishFailure(StorageResult::INTERNAL_ERROR);
}

StorageResult StorageRuntime::cancel()
{
    if (!initialized_) return StorageResult::NOT_INITIALIZED;
    if (!busy_) return StorageResult::INVALID_OPERATION;
    const StorageBackendResult backendResult = backend_.cancel();
    if (!isValidStorageBackendResult(backendResult))
        return finishFailure(StorageResult::INTERNAL_ERROR);
    if (backendResult == StorageBackendResult::SUCCESS ||
        backendResult == StorageBackendResult::CANCELLED)
    {
        transaction_.state_ = StorageOperationState::CANCELLED;
        transaction_.result_ = StorageResult::CANCELLED;
        busy_ = false;
        return StorageResult::CANCELLED;
    }
    if (backendResult == StorageBackendResult::IN_PROGRESS ||
        backendResult == StorageBackendResult::RETRY_LATER)
    {
        transaction_.state_ = StorageOperationState::RUNNING;
        transaction_.result_ = StorageResult::BACKEND_RETRY_LATER;
        return StorageResult::BACKEND_RETRY_LATER;
    }
    if (backendResult == StorageBackendResult::FAILED)
        return finishFailure(StorageResult::BACKEND_FAILED);
    return finishFailure(StorageResult::INTERNAL_ERROR);
}

const StorageTransaction& StorageRuntime::transaction() const { return transaction_; }

StorageResult StorageRuntime::clearCompleted()
{
    if (!initialized_) return StorageResult::NOT_INITIALIZED;
    if (busy_) return StorageResult::BUSY;
    if (transaction_.isValid() &&
        (transaction_.state() == StorageOperationState::PENDING ||
         transaction_.state() == StorageOperationState::RUNNING))
        return StorageResult::BUSY;
    transaction_ = StorageTransaction{};
    return StorageResult::SUCCESS;
}
