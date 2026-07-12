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

enum class IRDataType : uint8_t
{
    NONE = 0,
    DECODED,
    RAW
};

// Buffer ثابت است؛ Frame بزرگ‌تر برای جلوگیری از بازپخش ناقص رد می‌شود.
constexpr uint16_t IR_MAX_RAW_LENGTH = 220;

// پیام مستقل از Driver برای جابه‌جایی یک فرمان IR
struct IRMessage
{
    IRProtocol protocol = IRProtocol::UNKNOWN;

    uint64_t code = 0;
    uint16_t bits = 0;

    IRDataType dataType = IRDataType::NONE;

    // زمان‌های Mark و Space بر حسب میکروثانیه و مستقل از Buffer کتابخانه
    uint16_t rawData[IR_MAX_RAW_LENGTH] = {};
    uint16_t rawLength = 0;

    // گیرنده معمولی Carrier واقعی را اندازه‌گیری نمی‌کند؛ 38kHz پیش‌فرض بازپخش است.
    uint16_t carrierFrequencyKhz = 38;

    bool overflow = false;
    uint32_t timestampMs = 0;
    bool valid = false;
};

#endif
