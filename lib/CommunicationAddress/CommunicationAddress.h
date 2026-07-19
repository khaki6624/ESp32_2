#ifndef COMMUNICATION_ADDRESS_H
#define COMMUNICATION_ADDRESS_H
#include <CommunicationCommon.h>
class CommunicationAddress
{
public:
    CommunicationAddress();
    bool set(const char* value);
    bool isValid() const;
    const char* c_str() const;
    size_t length() const;
    bool equals(const CommunicationAddress& other) const;
private:
    char value_[COMMUNICATION_MAX_ADDRESS_LENGTH];
};
#endif
