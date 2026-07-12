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
    initialized = true;
}

bool IRSender::send(const IRMessage& message)
{
    if (!initialized)
        return false;

    if (!message.valid)
        return false;

    switch (message.dataType)
    {
        case IRDataType::DECODED:
            return sendDecoded(message);

        case IRDataType::RAW:
            return sendRawMessage(message);

        default:
            return false;
    }
}

bool IRSender::send(
    IRProtocol protocol,
    uint64_t code,
    uint16_t bits
)
{
    if (!initialized)
        return false;

    if (bits == 0 || bits > 64)
        return false;

    return sendDecoded(protocol, code, bits);
}

uint8_t IRSender::getPin() const
{
    return pin;
}

bool IRSender::sendDecoded(const IRMessage& message)
{
    if (message.dataType != IRDataType::DECODED || message.overflow)
        return false;

    return sendDecoded(message.protocol, message.code, message.bits);
}

bool IRSender::sendDecoded(
    IRProtocol protocol,
    uint64_t code,
    uint16_t bits
)
{
    if (bits == 0 || bits > 64 ||
        (bits < 64 && (code >> bits) != 0))
    {
        return false;
    }

    // تبدیل پروتکل داخلی پروژه به تابع ارسال متناظر در IRremoteESP8266
    switch (protocol)
    {
        case IRProtocol::NEC:
            if (bits != kNECBits)
                return false;
            sender.sendNEC(code, bits);
            return true;

        case IRProtocol::SONY:
            if (bits != kSony12Bits && bits != kSony15Bits &&
                bits != kSony20Bits)
            {
                return false;
            }
            sender.sendSony(code, bits);
            return true;

        case IRProtocol::SAMSUNG:
            if (bits != kSamsungBits)
                return false;
            sender.sendSAMSUNG(code, bits);
            return true;

        case IRProtocol::LG:
            if (bits != kLgBits && bits != kLg32Bits)
                return false;
            sender.sendLG(code, bits);
            return true;

        case IRProtocol::PANASONIC:
            if (bits != kPanasonic40Bits && bits != kPanasonicBits)
                return false;
            sender.sendPanasonic64(code, bits);
            return true;

        case IRProtocol::JVC:
            if (bits != kJvcBits)
                return false;
            sender.sendJVC(code, bits);
            return true;

        case IRProtocol::RC5:
            if (bits != kRC5Bits && bits != kRC5XBits)
                return false;
            sender.sendRC5(code, bits);
            return true;

        case IRProtocol::RC6:
            if (bits != kRC6Mode0Bits && bits != kRC6_36Bits)
                return false;
            sender.sendRC6(code, bits);
            return true;

        default:
            return false;
    }
}

bool IRSender::sendRawMessage(const IRMessage& message)
{
    // پیام ناقص یا overflow شده هرگز به سخت‌افزار ارسال نمی‌شود.
    if (message.dataType != IRDataType::RAW ||
        message.rawLength == 0 ||
        message.rawLength > IR_MAX_RAW_LENGTH ||
        message.overflow ||
        message.carrierFrequencyKhz == 0)
    {
        return false;
    }

    // rawData بر حسب میکروثانیه است و از Mark آغاز می‌شود.
    sender.sendRaw(
        message.rawData,
        message.rawLength,
        message.carrierFrequencyKhz
    );

    return true;
}
