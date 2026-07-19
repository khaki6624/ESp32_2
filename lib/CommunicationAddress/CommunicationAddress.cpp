#include "CommunicationAddress.h"
#include <string.h>
namespace
{
bool allowed(char value)
{
    return (value >= 'A' && value <= 'Z') || (value >= 'a' && value <= 'z') ||
        (value >= '0' && value <= '9') || value == '_' || value == '-' ||
        value == '.' || value == '/' || value == ':' || value == '+' || value == '@';
}
}
CommunicationAddress::CommunicationAddress() : value_{} {}
bool CommunicationAddress::set(const char* value)
{
    if (value == nullptr) return false;
    size_t length = 0U;
    while (length < sizeof(value_) && value[length] != '\0')
    { if (!allowed(value[length])) return false; ++length; }
    if (length == 0U || length >= sizeof(value_)) return false;
    char candidate[COMMUNICATION_MAX_ADDRESS_LENGTH] = {};
    memcpy(candidate, value, length); memcpy(value_, candidate, sizeof(value_)); return true;
}
bool CommunicationAddress::isValid() const
{
    if (value_[0] == '\0') return false;
    for (size_t i = 0U; i < sizeof(value_); ++i)
    { if (value_[i] == '\0') return true; if (!allowed(value_[i])) return false; }
    return false;
}
const char* CommunicationAddress::c_str() const { return value_; }
size_t CommunicationAddress::length() const
{ size_t n=0U; while(n<sizeof(value_) && value_[n]!='\0') ++n; return n; }
bool CommunicationAddress::equals(const CommunicationAddress& other) const
{ return isValid() && other.isValid() && memcmp(value_, other.value_, sizeof(value_)) == 0; }
