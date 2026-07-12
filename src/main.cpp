#include <Arduino.h>
#include <IRCommon.h>
#include <IRReceiver.h>
#include <IRSender.h>

namespace Pins
{
constexpr uint8_t BUTTON = 25;
constexpr uint8_t IR_RECEIVER = 27;
constexpr uint8_t IR_SENDER = 26;
}

namespace Timing
{
constexpr uint32_t DEBOUNCE_MS = 50;
}

IRReceiver receiver(Pins::IR_RECEIVER);
IRSender sender(Pins::IR_SENDER);
IRMessage lastMessage;

enum class TestState : uint8_t
{
    WAITING_CAPTURE,
    WAITING_IR,
    READY_TO_SEND
};

TestState testState = TestState::WAITING_CAPTURE;
bool stableButtonState = HIGH;
bool lastRawButtonState = HIGH;
uint32_t lastButtonChangeMs = 0;

void printHex64(uint64_t value)
{
    const uint32_t high = static_cast<uint32_t>(value >> 32);
    const uint32_t low = static_cast<uint32_t>(value);

    Serial.print("0x");
    if (high > 0)
    {
        Serial.print(high, HEX);
        char lowPart[9];
        snprintf(lowPart, sizeof(lowPart), "%08lX", static_cast<unsigned long>(low));
        Serial.print(lowPart);
    }
    else
    {
        Serial.print(low, HEX);
    }
}

void printMessage(const IRMessage& message)
{
    Serial.println();
    Serial.println("========== RECEIVED ==========");

    Serial.print("Protocol  : ");
    Serial.println(static_cast<uint8_t>(message.protocol));

    Serial.print("DataType  : ");
    Serial.println(static_cast<uint8_t>(message.dataType));

    Serial.print("Code      : ");
    printHex64(message.code);
    Serial.println();

    Serial.print("Bits      : ");
    Serial.println(message.bits);

    Serial.print("RawLength : ");
    Serial.println(message.rawLength);

    Serial.print("Overflow  : ");
    Serial.println(message.overflow ? "YES" : "NO");

    Serial.print("Carrier   : ");
    Serial.print(message.carrierFrequencyKhz);
    Serial.println(" kHz");

    Serial.print("Timestamp : ");
    Serial.println(message.timestampMs);

    if (message.dataType == IRDataType::RAW && message.rawLength > 0)
    {
        const uint16_t previewLength = message.rawLength < 8 ? message.rawLength : 8;
        Serial.print("RawPreview: ");
        for (uint16_t index = 0; index < previewLength; ++index)
        {
            if (index > 0)
                Serial.print(", ");
            Serial.print(message.rawData[index]);
        }
        Serial.println();
    }

    Serial.println("==============================");
}

void handleButtonPress()
{
    if (testState == TestState::WAITING_CAPTURE)
    {
        receiver.clear();
        Serial.println();
        Serial.println("Waiting for IR...");
        testState = TestState::WAITING_IR;
        return;
    }

    if (testState == TestState::READY_TO_SEND)
    {
        Serial.println();
        Serial.println("Sending...");
        Serial.println(sender.send(lastMessage) ? "SUCCESS" : "FAILED");
        testState = TestState::WAITING_CAPTURE;
        Serial.println("Press button to Capture.");
    }
}

void updateButton()
{
    const bool rawButtonState = digitalRead(Pins::BUTTON);
    const uint32_t now = millis();

    if (rawButtonState != lastRawButtonState)
    {
        lastRawButtonState = rawButtonState;
        lastButtonChangeMs = now;
    }

    if (rawButtonState != stableButtonState &&
        now - lastButtonChangeMs >= Timing::DEBOUNCE_MS)
    {
        stableButtonState = rawButtonState;
        if (stableButtonState == LOW)
            handleButtonPress();
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(Pins::BUTTON, INPUT_PULLUP);

    receiver.begin();
    sender.begin();

    Serial.println();
    Serial.println("========== DELSAM IR TEST ==========");
    Serial.println("Press button to Capture.");
}

void loop()
{
    updateButton();
    receiver.update();

    if (testState == TestState::WAITING_IR && receiver.available())
    {
        lastMessage = receiver.read();
        printMessage(lastMessage);
        testState = TestState::READY_TO_SEND;
        Serial.println("Press button to Send.");
    }
}
