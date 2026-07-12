#include "IRReceiver.h"

IRReceiver::IRReceiver(uint8_t gpio) :
    pin(gpio),
    receiver(gpio, IR_MAX_RAW_LENGTH),
    results{},
    pendingMessage{}
{
}

void IRReceiver::begin()
{
    // فعال‌سازی گیرنده IR
    receiver.enableIRIn();
    clear();
}

void IRReceiver::update()
{
    // بررسی غیرمسدودکننده برای دریافت سیگنال جدید
    if (!receiver.decode(&results))
        return;

    pendingMessage = IRMessage{};
    pendingMessage.protocol = mapProtocol(results.decode_type);
    // برای UNKNOWN، code و bits فقط اطلاعات کمکی Debug هستند؛ بازپخش واقعی از
    // rawData انجام می‌شود و لایه‌های بالاتر نباید برای RAW فقط به این دو تکیه کنند.
    pendingMessage.code = results.value;
    pendingMessage.bits = results.bits;
    pendingMessage.overflow = results.overflow;
    pendingMessage.timestampMs = millis();

    if (pendingMessage.protocol != IRProtocol::UNKNOWN)
    {
        pendingMessage.dataType = IRDataType::DECODED;
        pendingMessage.valid = !pendingMessage.overflow && pendingMessage.bits > 0;
    }
    else
    {
        pendingMessage.dataType = IRDataType::RAW;
        pendingMessage.valid = !pendingMessage.overflow && copyRawData();
    }

    // Buffer داخلی فقط پس از کپی کامل Timingها آزاد می‌شود.
    receiver.resume();
}

bool IRReceiver::available() const
{
    return pendingMessage.valid;
}

const IRMessage& IRReceiver::peek() const
{
    return pendingMessage;
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

IRProtocol IRReceiver::mapProtocol(decode_type_t protocol)
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

bool IRReceiver::copyRawData()
{
    // rawbuf[0] فاصله پیش از پیام است و بخشی از داده قابل ارسال نیست.
    if (results.rawbuf == nullptr || results.rawlen <= kStartOffset)
    {
        pendingMessage.rawLength = 0;
        return false;
    }

    uint16_t outputIndex = 0;

    for (uint16_t inputIndex = kStartOffset;
         inputIndex < results.rawlen;
         ++inputIndex)
    {
        uint32_t microseconds =
            static_cast<uint32_t>(results.rawbuf[inputIndex]) * kRawTick;

        // sendRaw ورودی uint16_t می‌گیرد؛ فاصله‌های بلند مطابق قرارداد
        // IRremoteESP8266 با Mark کامل و Space صفر به چند بخش شکسته می‌شوند.
        while (microseconds > UINT16_MAX)
        {
            if (outputIndex + 2 > IR_MAX_RAW_LENGTH)
            {
                pendingMessage.overflow = true;
                pendingMessage.rawLength = 0;
                return false;
            }

            pendingMessage.rawData[outputIndex++] = UINT16_MAX;
            pendingMessage.rawData[outputIndex++] = 0;
            microseconds -= UINT16_MAX;
        }

        if (outputIndex >= IR_MAX_RAW_LENGTH)
        {
            pendingMessage.overflow = true;
            pendingMessage.rawLength = 0;
            return false;
        }

        pendingMessage.rawData[outputIndex++] = static_cast<uint16_t>(microseconds);
    }

    pendingMessage.rawLength = outputIndex;
    return outputIndex > 0;
}
