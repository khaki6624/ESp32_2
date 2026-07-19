#ifndef STORAGE_REQUEST_H
#define STORAGE_REQUEST_H

#include <StorageBuffer.h>
#include <StorageKey.h>

class StorageRequest
{
public:
    StorageRequest();
    static StorageRequest read(StorageOperationId operationId,
        const StorageKey& key, const StorageWriteBuffer& output);
    static StorageRequest write(StorageOperationId operationId,
        const StorageKey& key, const StorageReadBuffer& input, bool overwrite);
    static StorageRequest remove(StorageOperationId operationId, const StorageKey& key);
    static StorageRequest exists(StorageOperationId operationId, const StorageKey& key);

    bool isValid() const;
    StorageOperationId operationId() const;
    StorageOperationType type() const;
    const StorageKey& key() const;
    const StorageReadBuffer& input() const;
    const StorageWriteBuffer& output() const;
    bool overwrite() const;

private:
    StorageOperationId operationId_;
    StorageOperationType type_;
    StorageKey key_;
    StorageReadBuffer input_;
    StorageWriteBuffer output_;
    bool overwrite_;
    bool valid_;
};

#endif
