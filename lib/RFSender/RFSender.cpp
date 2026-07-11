#include "RFSender.h"

RFSender::RFSender(uint8_t gpio) :
    pin(gpio),
    sender()
{
}

void RFSender::begin()
{
    // فعال‌سازی فرستنده RF روی پایه مشخص‌شده
    sender.enableTransmit(pin);
}

bool RFSender::send(const RFMessage& message)
{
    if (!message.valid)
        return false;

    return send(
        message.protocol,
        message.code,
        message.bits,
        message.pulseLength,
        message.repeatCount
    );
}

bool RFSender::send(
    RFProtocol protocol,
    uint64_t code,
    uint16_t bits,
    uint16_t pulseLength,
    uint8_t repeatCount
)
{
    uint8_t protocolNumber = mapProtocol(protocol);

    if (protocolNumber == 0 || bits == 0)
        return false;

    // RCSwitch در API ارسال عددی خود unsigned long می‌گیرد؛ روی ESP32 این مقدار ۳۲ بیت است.
    // برای جلوگیری از ارسال ناقص، کدهای بزرگ‌تر از ۳۲ بیت رد می‌شوند.
    if (code > 0xFFFFFFFFULL)
        return false;

    sender.setProtocol(protocolNumber);

    if (pulseLength > 0)
        sender.setPulseLength(pulseLength);

    if (repeatCount > 0)
        sender.setRepeatTransmit(repeatCount);

    sender.send(static_cast<unsigned long>(code), bits);

    return true;
}

uint8_t RFSender::getPin() const
{
    return pin;
}

uint8_t RFSender::mapProtocol(RFProtocol protocol) const
{
    // تبدیل پروتکل داخلی پروژه به شماره پروتکل RCSwitch
    switch (protocol)
    {
        case RFProtocol::PROTOCOL_1:  return 1;
        case RFProtocol::PROTOCOL_2:  return 2;
        case RFProtocol::PROTOCOL_3:  return 3;
        case RFProtocol::PROTOCOL_4:  return 4;
        case RFProtocol::PROTOCOL_5:  return 5;
        case RFProtocol::PROTOCOL_6:  return 6;
        case RFProtocol::PROTOCOL_7:  return 7;
        case RFProtocol::PROTOCOL_8:  return 8;
        case RFProtocol::PROTOCOL_9:  return 9;
        case RFProtocol::PROTOCOL_10: return 10;
        case RFProtocol::PROTOCOL_11: return 11;
        case RFProtocol::PROTOCOL_12: return 12;
        default:                      return 0;
    }
}
