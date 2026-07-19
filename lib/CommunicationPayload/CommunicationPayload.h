#ifndef COMMUNICATION_PAYLOAD_H
#define COMMUNICATION_PAYLOAD_H
#include <CommunicationCommon.h>
class CommunicationReadPayload
{
public:
    CommunicationReadPayload();
    CommunicationReadPayload(const uint8_t* data, size_t length);
    bool isValid() const; const uint8_t* data() const; size_t length() const;
private: const uint8_t* data_; size_t length_;
};
class CommunicationWritePayload
{
public:
    CommunicationWritePayload();
    CommunicationWritePayload(uint8_t* data, size_t capacity);
    bool isValid() const; uint8_t* data() const; size_t capacity() const;
private: uint8_t* data_; size_t capacity_;
};
#endif
