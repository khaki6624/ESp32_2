#ifndef STORAGE_BACKEND_H
#define STORAGE_BACKEND_H

#include <StorageRequest.h>

class StorageBackend
{
public:
    virtual ~StorageBackend() = default;
    virtual StorageBackendResult begin() = 0;
    virtual bool isReady() const = 0;
    virtual StorageBackendResult start(const StorageRequest& request) = 0;
    virtual StorageBackendResult update(size_t& transferredLength) = 0;
    virtual StorageBackendResult cancel() = 0;
};

// Bufferهای Request تا پایان عملیات باید نزد Caller زنده و معتبر بمانند.
// Runtime و Backend مالک این حافظه نیستند و Runtime از داده Copy نمی‌سازد.

#endif
