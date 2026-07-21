#include <EventDispatcher.h>
#include <EventNotificationFormatter.h>
#include <EventNotificationRegistry.h>
#include <EventQueue.h>
#include <EventRuntimeControllerPort.h>
#include <EventToNotificationAdapter.h>
#include <EventToNotificationBridge.h>
#include <EventToNotificationDispatchBinding.h>
#include <MainController.h>
#include <NotificationRegistry.h>
#include <NotificationRuntime.h>
#include <Arduino.h>
#include <IRCommon.h>
#include <IRReceiver.h>
#include <IRSender.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

namespace Pins
{
constexpr uint8_t BUTTON = 25;
constexpr uint8_t IR_RECEIVER = 27;
constexpr uint8_t IR_SENDER = 26;
constexpr uint8_t STATUS_LED = LED_BUILTIN;
}

namespace Timing
{
constexpr uint32_t DEBOUNCE_MS = 50;

// چشمک سریع هنگام انتظار برای دریافت فرمان کنترل
constexpr uint32_t LEARN_BLINK_MS = 150;

// چشمک آرام پس از دریافت فرمان و انتظار برای Send
constexpr uint32_t READY_BLINK_MS = 800;
}

// روی اغلب ESP32 DevKitها، LED داخلی با HIGH روشن می‌شود.
// اگر روی برد تو برعکس بود، مقدار را true کن.
constexpr bool STATUS_LED_ACTIVE_LOW = false;

namespace RuntimeComposition
{
class ProductionEventNotificationFormatter final : public EventNotificationFormatter
{
public:
    EventNotificationFormatResult format(
        const Event& event,
        uint8_t* output,
        size_t capacity,
        size_t& length) override
    {
        if (!event.isValid()) return EventNotificationFormatResult::INVALID_EVENT;
        if (output == nullptr || capacity < 1U)
            return EventNotificationFormatResult::BUFFER_TOO_SMALL;
        output[0] = static_cast<uint8_t>(event.type);
        length = 1U;
        return EventNotificationFormatResult::SUCCESS;
    }
};

NotificationRegistry notificationRegistry;
NotificationRuntime notificationRuntime(notificationRegistry);
ProductionEventNotificationFormatter notificationFormatter;
EventNotificationRegistry eventNotificationRegistry;

EventQueue eventQueue;
EventDispatcher eventDispatcher(eventQueue);
EventToNotificationAdapter eventToNotificationAdapter(
    eventNotificationRegistry,
    notificationFormatter,
    notificationRuntime);
EventToNotificationBridge eventToNotificationBridge(eventToNotificationAdapter);
EventToNotificationDispatchBinding eventDispatchBinding(
    eventQueue,
    eventDispatcher,
    eventToNotificationBridge);
EventRuntimeControllerPort eventRuntimePort(eventDispatchBinding);

MainControllerDependencies makeMainControllerDependencies()
{
    MainControllerDependencies dependencies;
    dependencies.set(eventRuntimePort);
    dependencies.require(MainControllerComponent::EVENT);
    return dependencies;
}

MainControllerDependencies mainControllerDependencies =
    makeMainControllerDependencies();
MainController mainController(mainControllerDependencies);

bool configure()
{
    return notificationRuntime.begin() == NotificationRuntimeResult::SUCCESS &&
        eventToNotificationAdapter.begin() == RuntimeIntegrationResult::SUCCESS &&
        mainController.begin(millis()) == MainControllerResult::SUCCESS;
}
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

// وضعیت کلید تست
bool stableButtonState = HIGH;
bool lastRawButtonState = HIGH;
uint32_t lastButtonChangeMs = 0;

// وضعیت LED
bool statusLedState = false;
uint32_t lastLedChangeMs = 0;

void setStatusLed(bool enabled)
{
    statusLedState = enabled;

    digitalWrite(
        Pins::STATUS_LED,
        enabled == STATUS_LED_ACTIVE_LOW ? LOW : HIGH
    );
}

void updateStatusLed()
{
    uint32_t blinkIntervalMs = 0;

    switch (testState)
    {
        case TestState::WAITING_CAPTURE:
            // هنوز Capture شروع نشده؛ LED خاموش بماند.
            if (statusLedState)
            {
                setStatusLed(false);
            }
            return;

        case TestState::WAITING_IR:
            // چشمک سریع هنگام انتظار برای دریافت فرمان IR
            blinkIntervalMs = Timing::LEARN_BLINK_MS;
            break;

        case TestState::READY_TO_SEND:
            // چشمک آرام پس از Learn و انتظار برای Send
            blinkIntervalMs = Timing::READY_BLINK_MS;
            break;
    }

    const uint32_t now = millis();

    if ((now - lastLedChangeMs) >= blinkIntervalMs)
    {
        lastLedChangeMs = now;
        setStatusLed(!statusLedState);
    }
}

void setTestState(TestState newState)
{
    testState = newState;

    // با تغییر State، الگوی LED از ابتدا آغاز شود.
    lastLedChangeMs = millis();

    switch (testState)
    {
        case TestState::WAITING_CAPTURE:
            setStatusLed(false);
            break;

        case TestState::WAITING_IR:
            // شروع فوری چشمک سریع
            setStatusLed(true);
            break;

        case TestState::READY_TO_SEND:
            // شروع فوری چشمک آرام
            setStatusLed(true);
            break;
    }
}

void printHex64(uint64_t value)
{
    const uint32_t high = static_cast<uint32_t>(value >> 32);
    const uint32_t low = static_cast<uint32_t>(value);

    Serial.print("0x");

    if (high > 0)
    {
        Serial.print(high, HEX);

        char lowPart[9];
        snprintf(
            lowPart,
            sizeof(lowPart),
            "%08lX",
            static_cast<unsigned long>(low)
        );

        Serial.print(lowPart);
    }
    else
    {
        Serial.print(low, HEX);
    }
}

void printRawPreview(const IRMessage& message)
{
    if (message.dataType != IRDataType::RAW ||
        message.rawLength == 0)
    {
        return;
    }

    const uint16_t previewLength =
        message.rawLength < 8 ? message.rawLength : 8;

    Serial.print("RawPreview: ");

    for (uint16_t index = 0; index < previewLength; ++index)
    {
        if (index > 0)
        {
            Serial.print(", ");
        }

        Serial.print(message.rawData[index]);
    }

    Serial.println();
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

    printRawPreview(message);

    Serial.println("==============================");
}

void handleButtonPress()
{
    if (testState == TestState::WAITING_CAPTURE)
    {
        receiver.clear();

        Serial.println();
        Serial.println("Waiting for IR...");
        Serial.println("Status LED: Fast blinking");

        setTestState(TestState::WAITING_IR);
        return;
    }

    if (testState == TestState::READY_TO_SEND)
    {
        Serial.println();
        Serial.println("Sending...");

        const bool sent = sender.send(lastMessage);

        Serial.println(sent ? "SUCCESS" : "FAILED");

        setTestState(TestState::WAITING_CAPTURE);

        Serial.println("Status LED: OFF");
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
        (now - lastButtonChangeMs) >= Timing::DEBOUNCE_MS)
    {
        stableButtonState = rawButtonState;

        // کلید بین GPIO25 و GND است.
        if (stableButtonState == LOW)
        {
            handleButtonPress();
        }
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(Pins::BUTTON, INPUT_PULLUP);
    pinMode(Pins::STATUS_LED, OUTPUT);

    setStatusLed(false);

    receiver.begin();
    sender.begin();

    if (!RuntimeComposition::configure())
    {
        Serial.println("Runtime Integration initialization failed.");
    }

    Serial.println();
    Serial.println("========== DELSAM IR TEST ==========");
    Serial.print("Status LED GPIO: ");
    Serial.println(Pins::STATUS_LED);
    Serial.println("Press button to Capture.");
}

void loop()
{
    RuntimeComposition::mainController.update(millis());

    updateButton();
    updateStatusLed();

    receiver.update();

    if (testState == TestState::WAITING_IR &&
        receiver.available())
    {
        lastMessage = receiver.read();

        printMessage(lastMessage);

        Serial.println("IR Learned.");
        Serial.println("Status LED: Slow blinking");
        Serial.println("Press button to Send.");

        setTestState(TestState::READY_TO_SEND);
    }
}
