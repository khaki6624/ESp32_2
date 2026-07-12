#include "IRSender.h"

IRSender::IRSender(uint8_t gpio) :
    pin(gpio),
    sender(gpio)
{
}

void IRSender::begin()
{
    // فعال‌سازی فرستنده IR روی پایه مشخص‌شده
    sender.begin();
}

bool IRSender::send(const IRMessage& message)
{
    if (!message.valid)
        return false;

    return send(message.protocol, message.code, message.bits);
}

bool IRSender::send(
    IRProtocol protocol,
    uint64_t code,
    uint16_t bits
)
{
    if (bits == 0)
        return false;

    return sendProtocol(protocol, code, bits);
}

uint8_t IRSender::getPin() const
{
    return pin;
}

bool IRSender::sendProtocol(
    IRProtocol protocol,
    uint64_t code,
    uint16_t bits
)
{
    // تبدیل پروتکل داخلی پروژه به تابع ارسال متناظر در IRremoteESP8266
    switch (protocol)
    {
        case IRProtocol::NEC:
            sender.sendNEC(code, bits);
            return true;

        case IRProtocol::SONY:
            sender.sendSony(code, bits);
            return true;

        case IRProtocol::SAMSUNG:
            sender.sendSAMSUNG(code, bits);
            return true;

        case IRProtocol::LG:
            sender.sendLG(code, bits);
            return true;

        case IRProtocol::PANASONIC:
            sender.sendPanasonic64(code, bits);
            return true;

        case IRProtocol::JVC:
            sender.sendJVC(code, bits);
            return true;

        case IRProtocol::RC5:
            sender.sendRC5(code, bits);
            return true;

        case IRProtocol::RC6:
            sender.sendRC6(code, bits);
            return true;

        default:
            return false;
    }
}
