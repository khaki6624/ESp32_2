#include "RFReceiver.h"

RFReceiver::RFReceiver(uint8_t gpio) :
    pin(gpio),
    receiver(),
    lastMessage{}
{
}

void RFReceiver::begin()
{
    // فعال‌سازی گیرنده RF روی پایه مشخص‌شده
    receiver.enableReceive(pin);

    // پاک کردن پیام قبلی احتمالی
    clear();
}

void RFReceiver::update()
{
    // بررسی غیرمسدودکننده برای دریافت سیگنال جدید
    if (receiver.available())
    {
        // فعلاً صف تک‌عضوی داریم؛ اگر پیام قبلی خوانده نشده باشد، پیام جدید جایگزین آن می‌شود.
        lastMessage.protocol = mapProtocol(static_cast<uint8_t>(receiver.getReceivedProtocol()));
        lastMessage.code = static_cast<uint64_t>(receiver.getReceivedValue());
        lastMessage.bits = static_cast<uint16_t>(receiver.getReceivedBitlength());
        lastMessage.pulseLength = static_cast<uint16_t>(receiver.getReceivedDelay());
        lastMessage.repeatCount = 0;
        lastMessage.timestampMs = millis();
        lastMessage.valid = true;

        // آماده‌سازی کتابخانه برای دریافت پیام بعدی
        receiver.resetAvailable();
    }
}

bool RFReceiver::available() const
{
    return lastMessage.valid;
}

const RFMessage& RFReceiver::peek() const
{
    // مشاهده پیام بدون مصرف کردن آن
    return lastMessage;
}

RFMessage RFReceiver::read()
{
    // خواندن پیام، آن را از Driver خارج می‌کند
    RFMessage message = lastMessage;

    clear();

    return message;
}

void RFReceiver::clear()
{
    // بازنشانی پیام به حالت نامعتبر
    lastMessage = RFMessage{};
}

uint8_t RFReceiver::getPin() const
{
    return pin;
}

RFProtocol RFReceiver::mapProtocol(uint8_t protocol)
{
    // تبدیل شماره پروتکل کتابخانه RCSwitch به پروتکل داخلی پروژه
    switch (protocol)
    {
        case 1:  return RFProtocol::PROTOCOL_1;
        case 2:  return RFProtocol::PROTOCOL_2;
        case 3:  return RFProtocol::PROTOCOL_3;
        case 4:  return RFProtocol::PROTOCOL_4;
        case 5:  return RFProtocol::PROTOCOL_5;
        case 6:  return RFProtocol::PROTOCOL_6;
        case 7:  return RFProtocol::PROTOCOL_7;
        case 8:  return RFProtocol::PROTOCOL_8;
        case 9:  return RFProtocol::PROTOCOL_9;
        case 10: return RFProtocol::PROTOCOL_10;
        case 11: return RFProtocol::PROTOCOL_11;
        case 12: return RFProtocol::PROTOCOL_12;
        default: return RFProtocol::UNKNOWN;
    }
}
