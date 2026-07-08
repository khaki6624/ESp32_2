#include "IRReceiver.h"

IRReceiver::IRReceiver(uint8_t gpio) :
    pin(gpio),
    receiver(new IRrecv(gpio)),
    results{},
    pendingMessage{}
{
}

IRReceiver::~IRReceiver()
{
    // آزاد کردن حافظه اختصاص‌داده‌شده به گیرنده
    if (receiver != nullptr)
    {
        delete receiver;
        receiver = nullptr;
    }
}

void IRReceiver::begin()
{
    // فعال‌سازی گیرنده IR
    receiver->enableIRIn();
    clear();
}

void IRReceiver::update()
{
    // بررسی غیرمسدودکننده برای دریافت سیگنال جدید
    if (receiver->decode(&results))
    {
        pendingMessage.protocol = mapProtocol(results.decode_type);
        pendingMessage.code = results.value;
        pendingMessage.bits = results.bits;
        pendingMessage.valid = true;

        // آماده‌سازی گیرنده برای دریافت سیگنال بعدی
        receiver->resume();
    }
}

bool IRReceiver::available() const
{
    return pendingMessage.valid;
}

IRMessage IRReceiver::read()
{
    // خواندن پیام، آن را از صف تک‌عضوی Driver خارج می‌کند.
    IRMessage message = pendingMessage;
    clear();
    return message;
}

void IRReceiver::clear()
{
    pendingMessage = IRMessage{};
}

uint8_t IRReceiver::getPin() const
{
    return pin;
}

IRProtocol IRReceiver::mapProtocol(decode_type_t protocol) const
{
    switch (protocol)
    {
        case NEC:       return IRProtocol::NEC;
        case SONY:      return IRProtocol::SONY;
        case SAMSUNG:   return IRProtocol::SAMSUNG;
        case LG:        return IRProtocol::LG;
        case PANASONIC: return IRProtocol::PANASONIC;
        case JVC:       return IRProtocol::JVC;
        case RC5:       return IRProtocol::RC5;
        case RC6:       return IRProtocol::RC6;
        default:        return IRProtocol::UNKNOWN;
    }
}
