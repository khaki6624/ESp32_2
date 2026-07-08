#ifndef IR_COMMON_H
#define IR_COMMON_H

#include <Arduino.h>

// پروتکل‌های مشترک بین گیرنده و فرستنده IR
enum class IRProtocol : uint8_t
{
    UNKNOWN = 0,
    NEC,
    SONY,
    SAMSUNG,
    LG,
    PANASONIC,
    JVC,
    RC5,
    RC6
};

// پیام مستقل از Driver برای جابه‌جایی یک فرمان IR
struct IRMessage
{
    IRProtocol protocol = IRProtocol::UNKNOWN;
    uint64_t code = 0;
    uint16_t bits = 0;
    bool valid = false;
};

#endif
