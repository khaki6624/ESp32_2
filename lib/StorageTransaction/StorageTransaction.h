#ifndef STORAGE_TRANSACTION_H
#define STORAGE_TRANSACTION_H

#include <StorageCommon.h>

class StorageRuntime;

class StorageTransaction
{
public:
    StorageTransaction();
    bool isValid() const;
    StorageOperationId operationId() const;
    StorageOperationType type() const;
    StorageOperationState state() const;
    StorageResult result() const;
    size_t transferredLength() const;

private:
    friend class StorageRuntime;
    StorageOperationId operationId_;
    StorageOperationType type_;
    StorageOperationState state_;
    StorageResult result_;
    size_t transferredLength_;
    bool valid_;
};

#endif
