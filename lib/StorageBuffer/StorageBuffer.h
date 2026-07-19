#ifndef STORAGE_BUFFER_H
#define STORAGE_BUFFER_H

#include <StorageCommon.h>

class StorageReadBuffer
{
public:
    StorageReadBuffer();
    StorageReadBuffer(const uint8_t* data, size_t length);
    bool isValid() const;
    const uint8_t* data() const;
    size_t length() const;

private:
    const uint8_t* data_;
    size_t length_;
};

class StorageWriteBuffer
{
public:
    StorageWriteBuffer();
    StorageWriteBuffer(uint8_t* data, size_t capacity);
    bool isValid() const;
    uint8_t* data() const;
    size_t capacity() const;

private:
    uint8_t* data_;
    size_t capacity_;
};

#endif
