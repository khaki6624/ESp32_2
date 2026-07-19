#include "StorageRequest.h"

StorageRequest::StorageRequest() : operationId_(INVALID_STORAGE_OPERATION_ID),
    type_(StorageOperationType::COUNT), key_{}, input_{}, output_{},
    overwrite_(false), valid_(false) {}

StorageRequest StorageRequest::read(StorageOperationId operationId,
    const StorageKey& key, const StorageWriteBuffer& output)
{
    StorageRequest result;
    if (operationId == INVALID_STORAGE_OPERATION_ID || !key.isValid() || !output.isValid())
        return result;
    result.operationId_ = operationId;
    result.type_ = StorageOperationType::READ;
    result.key_ = key;
    result.output_ = output;
    result.valid_ = true;
    return result;
}

StorageRequest StorageRequest::write(StorageOperationId operationId,
    const StorageKey& key, const StorageReadBuffer& input, bool overwrite)
{
    StorageRequest result;
    if (operationId == INVALID_STORAGE_OPERATION_ID || !key.isValid() || !input.isValid())
        return result;
    result.operationId_ = operationId;
    result.type_ = StorageOperationType::WRITE;
    result.key_ = key;
    result.input_ = input;
    result.overwrite_ = overwrite;
    result.valid_ = true;
    return result;
}

StorageRequest StorageRequest::remove(StorageOperationId operationId, const StorageKey& key)
{
    StorageRequest result;
    if (operationId == INVALID_STORAGE_OPERATION_ID || !key.isValid()) return result;
    result.operationId_ = operationId;
    result.type_ = StorageOperationType::REMOVE;
    result.key_ = key;
    result.valid_ = true;
    return result;
}

StorageRequest StorageRequest::exists(StorageOperationId operationId, const StorageKey& key)
{
    StorageRequest result;
    if (operationId == INVALID_STORAGE_OPERATION_ID || !key.isValid()) return result;
    result.operationId_ = operationId;
    result.type_ = StorageOperationType::EXISTS;
    result.key_ = key;
    result.valid_ = true;
    return result;
}

bool StorageRequest::isValid() const
{
    if (!valid_ || operationId_ == INVALID_STORAGE_OPERATION_ID ||
        !isValidStorageOperationType(type_) || !key_.isValid()) return false;
    if (type_ == StorageOperationType::READ)
        return output_.isValid() && !input_.isValid() && !overwrite_;
    if (type_ == StorageOperationType::WRITE)
        return input_.isValid() && !output_.isValid();
    return !input_.isValid() && !output_.isValid() && !overwrite_;
}

StorageOperationId StorageRequest::operationId() const { return operationId_; }
StorageOperationType StorageRequest::type() const { return type_; }
const StorageKey& StorageRequest::key() const { return key_; }
const StorageReadBuffer& StorageRequest::input() const { return input_; }
const StorageWriteBuffer& StorageRequest::output() const { return output_; }
bool StorageRequest::overwrite() const { return overwrite_; }
