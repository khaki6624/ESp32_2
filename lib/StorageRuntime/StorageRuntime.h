#ifndef STORAGE_RUNTIME_H
#define STORAGE_RUNTIME_H

#include <StorageBackend.h>
#include <StorageTransaction.h>

class StorageRuntime
{
public:
    explicit StorageRuntime(StorageBackend& backend);
    StorageResult begin();
    bool isInitialized() const;
    bool isBusy() const;
    StorageResult submit(const StorageRequest& request);
    StorageResult update();
    StorageResult cancel();
    const StorageTransaction& transaction() const;
    StorageResult clearCompleted();

private:
    StorageBackend& backend_;
    StorageRequest activeRequest_;
    StorageTransaction transaction_;
    bool initialized_;
    bool busy_;

    void setTransaction(const StorageRequest& request, StorageOperationState state,
        StorageResult result, size_t transferredLength);
    StorageResult finishFailure(StorageResult result);
    bool isTransferredLengthValid(const StorageRequest& request,
        size_t transferredLength) const;
};

#endif
