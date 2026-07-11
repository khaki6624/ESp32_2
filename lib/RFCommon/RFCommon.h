#ifndef RF_COMMON_H
#define RF_COMMON_H

#include <Arduino.h>

// پروتکل‌های مشترک بین گیرنده و فرستنده RF
enum class RFProtocol : uint8_t
{
    UNKNOWN = 0,
    PROTOCOL_1,
    PROTOCOL_2,
    PROTOCOL_3,
    PROTOCOL_4,
    PROTOCOL_5,
    PROTOCOL_6,
    PROTOCOL_7,
    PROTOCOL_8,
    PROTOCOL_9,
    PROTOCOL_10,
    PROTOCOL_11,
    PROTOCOL_12
};

// پیام مستقل از Driver برای جابه‌جایی یک فرمان RF
struct RFMessage
{
    RFProtocol protocol = RFProtocol::UNKNOWN;
    uint64_t code = 0;
    uint16_t bits = 0;
    uint16_t pulseLength = 0;
    uint8_t repeatCount = 0;
    bool valid = false;
};

#endif
