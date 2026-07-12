#include <Arduino.h>

// باید قبل از IRremote.hpp تعریف شود.
#define RAW_BUFFER_LENGTH 750
#define IR_SEND_PIN 26

#include <IRremote.hpp>

namespace Pins {
constexpr uint8_t BUTTON = 25;
constexpr uint8_t IR_RECEIVER = 27;
constexpr uint8_t IR_SENDER = 26;
}

namespace Timing {
constexpr uint32_t DEBOUNCE_MS = 50;
}

enum class TestState : uint8_t {
    WaitingForCaptureTrigger,
    WaitingForIRSignal,
    ReadyToSend
};

struct StoredIRData {
    IRData decodedData {};

    uint8_t rawData[RAW_BUFFER_LENGTH] {};
    uint16_t rawLength = 0;

    bool valid = false;
    bool isRaw = false;
};

StoredIRData storedIR;
TestState testState = TestState::WaitingForCaptureTrigger;

bool lastRawButtonState = HIGH;
bool stableButtonState = HIGH;
uint32_t lastButtonChangeTime = 0;

bool buttonPressed();
bool storeReceivedIR();
void printStoredIR();
void sendStoredIR();

void setup() {
    Serial.begin(115200);

    pinMode(Pins::BUTTON, INPUT_PULLUP);

    IrReceiver.begin(
        Pins::IR_RECEIVER,
        DISABLE_LED_FEEDBACK
    );

    IrSender.begin(Pins::IR_SENDER);

    Serial.println();
    Serial.println("================================");
    Serial.println("Delsam IR Receiver/Sender Test");
    Serial.println("================================");
    Serial.print("IR receiver GPIO: ");
    Serial.println(Pins::IR_RECEIVER);

    Serial.print("IR sender GPIO: ");
    Serial.println(Pins::IR_SENDER);

    Serial.print("Test button GPIO: ");
    Serial.println(Pins::BUTTON);

    Serial.println();
    Serial.println("Step 1: Press the GPIO25 test button.");
    Serial.println("Step 2: Point the remote at the receiver.");
    Serial.println("Step 3: Press one remote button.");
    Serial.println("Step 4: Press GPIO25 again to transmit it.");
}

void loop() {
    if (buttonPressed()) {
        switch (testState) {
            case TestState::WaitingForCaptureTrigger:
                storedIR.valid = false;
                storedIR.isRaw = false;
                storedIR.rawLength = 0;

                testState = TestState::WaitingForIRSignal;

                Serial.println();
                Serial.println("--------------------------------");
                Serial.println("Capture mode enabled.");
                Serial.println("Press one button on the IR remote...");
                Serial.println("--------------------------------");
                break;

            case TestState::WaitingForIRSignal:
                Serial.println();
                Serial.println("Still waiting for an IR signal...");
                break;

            case TestState::ReadyToSend:
                sendStoredIR();
                break;
        }
    }

    if (testState == TestState::WaitingForIRSignal &&
        IrReceiver.decode()) {

        if (storeReceivedIR()) {
            testState = TestState::ReadyToSend;

            Serial.println();
            Serial.println("--------------------------------");
            Serial.println("IR command stored successfully.");
            Serial.println("Press GPIO25 to transmit it.");
            Serial.println("--------------------------------");
        }

        IrReceiver.resume();
    }
}

bool buttonPressed() {
    const bool rawState = digitalRead(Pins::BUTTON);

    if (rawState != lastRawButtonState) {
        lastRawButtonState = rawState;
        lastButtonChangeTime = millis();
    }

    if (millis() - lastButtonChangeTime < Timing::DEBOUNCE_MS) {
        return false;
    }

    if (rawState == stableButtonState) {
        return false;
    }

    stableButtonState = rawState;

    // کلید با INPUT_PULLUP به GND وصل است؛ بنابراین هنگام فشار LOW می‌شود.
    return stableButtonState == LOW;
}

bool storeReceivedIR() {
    const IRData &received = IrReceiver.decodedIRData;

    if (received.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
        Serial.println("ERROR: IR receive buffer overflow.");
        Serial.println("Increase RAW_BUFFER_LENGTH.");
        return false;
    }

    if (received.flags & IRDATA_FLAGS_IS_REPEAT) {
        Serial.println("Repeat frame ignored.");
        return false;
    }

    if (received.flags & IRDATA_FLAGS_IS_AUTO_REPEAT) {
        Serial.println("Automatic repeat frame ignored.");
        return false;
    }

    if (IrReceiver.irparams.rawlen < 4) {
        Serial.println("IR frame is too short and was ignored.");
        return false;
    }

    storedIR.decodedData = received;

    const decode_type_t protocol = received.protocol;

    storedIR.isRaw =
        protocol == UNKNOWN ||
        protocol == PULSE_WIDTH ||
        protocol == PULSE_DISTANCE;

    if (storedIR.isRaw) {
        storedIR.rawLength = IrReceiver.irparams.rawlen - 1;

        if (storedIR.rawLength > RAW_BUFFER_LENGTH) {
            Serial.println("ERROR: Received raw frame is too large.");
            return false;
        }

        IrReceiver.compensateAndStoreIRResultInArray(
            storedIR.rawData
        );
    }

    storedIR.decodedData.flags = 0;
    storedIR.valid = true;

    printStoredIR();
    return true;
}

void printStoredIR() {
    Serial.println();
    Serial.println("========== IR RECEIVED ==========");

    // خروجی خلاصه استاندارد کتابخانه
    IrReceiver.printIRResultShort(&Serial);

    Serial.print("Protocol: ");
    Serial.println(getProtocolString(storedIR.decodedData.protocol));

    Serial.print("Address: 0x");
    Serial.println(storedIR.decodedData.address, HEX);

    Serial.print("Command: 0x");
    Serial.println(storedIR.decodedData.command, HEX);

    Serial.print("Raw data: 0x");
    Serial.println(
        static_cast<uint64_t>(storedIR.decodedData.decodedRawData),
        HEX
    );

    Serial.print("Number of bits: ");
    Serial.println(storedIR.decodedData.numberOfBits);

    if (storedIR.isRaw) {
        Serial.print("Stored as RAW timing data. Entries: ");
        Serial.println(storedIR.rawLength);

        IrReceiver.printIRResultRawFormatted(
            &Serial,
            true
        );
    } else {
        Serial.println("Stored as a recognized IR protocol.");

        Serial.println("Suggested send command:");
        IrReceiver.printIRSendUsage(&Serial);
    }

    Serial.println("=================================");
}

void sendStoredIR() {
    if (!storedIR.valid) {
        Serial.println("No IR command has been captured.");
        return;
    }

    Serial.println();
    Serial.println("Stopping receiver...");
    IrReceiver.stop();

    Serial.println("Transmitting stored IR command...");
    Serial.flush();

    if (storedIR.isRaw) {
        // برای پروتکل ناشناخته، فرکانس متداول 38 کیلوهرتز فرض شده است.
        IrSender.sendRaw(
            storedIR.rawData,
            storedIR.rawLength,
            38
        );

        Serial.print("RAW IR sent. Timing entries: ");
        Serial.println(storedIR.rawLength);
    } else {
        // پروتکل، آدرس و فرمان ذخیره‌شده را مجدداً ارسال می‌کند.
        IrSender.write(&storedIR.decodedData);

        Serial.print("Protocol sent: ");
        Serial.println(
            getProtocolString(storedIR.decodedData.protocol)
        );

        Serial.print("Address: 0x");
        Serial.println(storedIR.decodedData.address, HEX);

        Serial.print("Command: 0x");
        Serial.println(storedIR.decodedData.command, HEX);
    }

    Serial.println("Transmission completed.");

    IrReceiver.start();
    Serial.println("Receiver restarted.");
}