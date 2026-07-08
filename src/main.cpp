#include <Arduino.h>
#include <BoardConfig.h>
#include <DeviceIdentity.h>
#include <IRReceiver.h>
#include <IRSender.h>

IRReceiver irReceiver(BoardConfig::IR_RECEIVER_PIN);
IRSender irSender(BoardConfig::IR_SENDER_PIN);

const char* protocolName(IRProtocol protocol)
{
    switch (protocol)
    {
        case IRProtocol::NEC:       return "NEC";
        case IRProtocol::SONY:      return "SONY";
        case IRProtocol::SAMSUNG:   return "SAMSUNG";
        case IRProtocol::LG:        return "LG";
        case IRProtocol::PANASONIC: return "PANASONIC";
        case IRProtocol::JVC:       return "JVC";
        case IRProtocol::RC5:       return "RC5";
        case IRProtocol::RC6:       return "RC6";
        default:                    return "UNKNOWN";
    }
}

void setup()
{
    Serial.begin(115200);

    Serial.println(DeviceIdentity::DEVICE_NAME);
    irReceiver.begin();
    irSender.begin();
}

void loop()
{
    irReceiver.update();

    if (irReceiver.available())
    {
        IRMessage message = irReceiver.read();

        Serial.print("Protocol: ");
        Serial.println(protocolName(message.protocol));
        Serial.print("Code: 0x");
        Serial.println(static_cast<uint64_t>(message.code), HEX);
        Serial.print("Bits: ");
        Serial.println(message.bits);
        Serial.print("Valid: ");
        Serial.println(message.valid ? "true" : "false");

        // ارسال دوباره همان پیام دریافت‌شده
        irSender.send(message);
    }

}
