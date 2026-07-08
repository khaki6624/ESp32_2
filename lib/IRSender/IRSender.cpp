#include "IRSender.h"

IRSender::IRSender(uint8_t gpio) : pin(gpio), sender(new IRsend(gpio))
{
}

IRSender::~IRSender()
{
    delete sender;
    sender = nullptr;
}

void IRSender::begin()
{
    sender->begin();
}

bool IRSender::send(const IRMessage& message)
{
    if (!message.valid)
        return false;

    return send(message.protocol, message.code, message.bits);
}

bool IRSender::send(IRProtocol protocol, uint64_t code, uint16_t bits)
{
    // Driver فقط پیام را به API متناظر کتابخانه IR تبدیل می‌کند.
    switch (protocol)
    {
        case IRProtocol::NEC:
            sender->sendNEC(code, bits);
            break;
        case IRProtocol::SONY:
            sender->sendSony(code, bits);
            break;
        case IRProtocol::SAMSUNG:
            sender->sendSAMSUNG(code, bits);
            break;
        case IRProtocol::LG:
            sender->sendLG(code, bits);
            break;
        case IRProtocol::PANASONIC:
            sender->sendPanasonic64(code, bits);
            break;
        case IRProtocol::JVC:
            sender->sendJVC(code, bits, false);
            break;
        case IRProtocol::RC5:
            sender->sendRC5(code, bits);
            break;
        case IRProtocol::RC6:
            sender->sendRC6(code, bits);
            break;
        case IRProtocol::UNKNOWN:
        default:
            return false;
    }

    return true;
}

uint8_t IRSender::getPin() const
{
    return pin;
}
