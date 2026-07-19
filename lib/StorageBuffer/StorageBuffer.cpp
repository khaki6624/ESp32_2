#include "StorageBuffer.h"

StorageReadBuffer::StorageReadBuffer() : data_(nullptr), length_(0U) {}
StorageReadBuffer::StorageReadBuffer(const uint8_t* data, size_t length) :
    data_(data), length_(length) {}
bool StorageReadBuffer::isValid() const
{ return data_ != nullptr && length_ > 0U && length_ <= STORAGE_MAX_DATA_LENGTH; }
const uint8_t* StorageReadBuffer::data() const { return data_; }
size_t StorageReadBuffer::length() const { return length_; }

StorageWriteBuffer::StorageWriteBuffer() : data_(nullptr), capacity_(0U) {}
StorageWriteBuffer::StorageWriteBuffer(uint8_t* data, size_t capacity) :
    data_(data), capacity_(capacity) {}
bool StorageWriteBuffer::isValid() const
{ return data_ != nullptr && capacity_ > 0U && capacity_ <= STORAGE_MAX_DATA_LENGTH; }
uint8_t* StorageWriteBuffer::data() const { return data_; }
size_t StorageWriteBuffer::capacity() const { return capacity_; }
