#include "IRReceiver.h"

IRReceiver::IRReceiver(uint8_t gpio) :
    pin(gpio),
    receiver(gpio),
    results{},
    lastMessage{}
{
}

void IRReceiver::begin()
{
    // فعال‌سازی گیرنده IR
    receiver.enableIRIn();

    // پاک کردن پیام قبلی احتمالی
    clear();
}

void IRReceiver::update()
{
    // بررسی غیرمسدودکننده برای دریافت سیگنال جدید
    if (receiver.decode(&results))
    {
        lastMessage.protocol = mapProtocol(results.decode_type);
        lastMessage.code = results.value;
        lastMessage.bits = results.bits;
        lastMessage.valid = true;

        // آماده‌سازی گیرنده برای دریافت سیگنال بعدی
        receiver.resume();
    }
}

bool IRReceiver::available() const
{
    return lastMessage.valid;
}

const IRMessage& IRReceiver::peek() const
{
    // مشاهده پیام بدون مصرف کردن آن
    return lastMessage;
}

IRMessage IRReceiver::read()
{
    // خواندن پیام، آن را از Driver خارج می‌کند
    IRMessage message = lastMessage;

    clear();

    return message;
}

void IRReceiver::clear()
{
    // بازنشانی پیام به حالت نامعتبر
    lastMessage = IRMessage{};
}

uint8_t IRReceiver::getPin() const
{
    return pin;
}

IRProtocol IRReceiver::mapProtocol(decode_type_t protocol) const
{
    // تبدیل پروتکل کتابخانه IRremoteESP8266 به پروتکل داخلی پروژه
    switch (protocol)
    {
        case NEC:
            return IRProtocol::NEC;

        case SONY:
            return IRProtocol::SONY;

        case SAMSUNG:
            return IRProtocol::SAMSUNG;

        case LG:
            return IRProtocol::LG;

        case PANASONIC:
            return IRProtocol::PANASONIC;

        case JVC:
            return IRProtocol::JVC;

        case RC5:
            return IRProtocol::RC5;

        case RC6:
            return IRProtocol::RC6;

        default:
            return IRProtocol::UNKNOWN;
    }
}