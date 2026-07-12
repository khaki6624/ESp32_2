#include <Arduino.h>
#include <Relay.h>

// ------------------------------
// تنظیمات تست
// ------------------------------
constexpr uint8_t RELAY_PIN = 23;
constexpr uint8_t TEST_BUTTON_PIN = 25;

constexpr bool RELAY_ACTIVE_LOW = true;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 50;
constexpr uint32_t RELAY_PULSE_MS = 2000;

// رله کانال ۱
Relay relay(RELAY_PIN, RELAY_ACTIVE_LOW);

// وضعیت کلید
bool lastRawButtonState = HIGH;
bool stableButtonState = HIGH;
uint32_t lastButtonChangeTime = 0;

// شماره مرحله تست
uint8_t testStep = 0;

void printHeader()
{
    Serial.println();
    Serial.println("========================================");
    Serial.println("        DELSAM HARDWARE TEST");
    Serial.println("========================================");
    Serial.println("Driver      : Relay");
    Serial.print("Relay GPIO  : ");
    Serial.println(RELAY_PIN);
    Serial.println("Relay Mode  : Active LOW");
    Serial.print("Test Button : GPIO");
    Serial.println(TEST_BUTTON_PIN);
    Serial.println("----------------------------------------");
    Serial.println("هر بار کلید را فشار بده تا مرحله بعد اجرا شود.");
    Serial.println("========================================");
    Serial.println();
}

void printRelayState()
{
    Serial.print("Relay state: ");
    Serial.println(relay.isOn() ? "ON" : "OFF");
}

void runNextTestStep()
{
    testStep++;

    if (testStep > 7)
    {
        testStep = 1;
        Serial.println();
        Serial.println("========== TEST CYCLE RESTARTED ==========");
    }

    Serial.println();
    Serial.print("Button pressed - Test step ");
    Serial.println(testStep);

    switch (testStep)
    {
        case 1:
            Serial.println("Action: relay.on()");
            relay.on();
            printRelayState();
            break;

        case 2:
            Serial.println("Action: relay.off()");
            relay.off();
            printRelayState();
            break;

        case 3:
            Serial.println("Action: relay.toggle()");
            relay.toggle();
            printRelayState();
            break;

        case 4:
            Serial.println("Action: relay.toggle()");
            relay.toggle();
            printRelayState();
            break;

        case 5:
            Serial.println("Action: relay.setState(true)");
            relay.setState(true);
            printRelayState();
            break;

        case 6:
            Serial.println("Action: relay.setState(false)");
            relay.setState(false);
            printRelayState();
            break;

        case 7:
            Serial.print("Action: relay.pulse(");
            Serial.print(RELAY_PULSE_MS);
            Serial.println(" ms)");
            relay.pulse(RELAY_PULSE_MS);

            Serial.println("Relay should turn ON now...");
            printRelayState();
            Serial.println("It should turn OFF automatically after 2 seconds.");
            break;

        default:
            break;
    }
}

bool wasTestButtonPressed()
{
    const bool rawState = digitalRead(TEST_BUTTON_PIN);
    const uint32_t now = millis();

    // هر تغییر خام، تایمر Debounce را از نو آغاز می‌کند.
    if (rawState != lastRawButtonState)
    {
        lastRawButtonState = rawState;
        lastButtonChangeTime = now;
    }

    // تا پایدار شدن کلید صبر می‌کنیم.
    if ((now - lastButtonChangeTime) < BUTTON_DEBOUNCE_MS)
    {
        return false;
    }

    // فقط تغییر وضعیت پایدار را پردازش می‌کنیم.
    if (stableButtonState != rawState)
    {
        stableButtonState = rawState;

        // کلید بین GPIO25 و GND است؛ بنابراین LOW یعنی فشرده شده.
        if (stableButtonState == LOW)
        {
            return true;
        }
    }

    return false;
}

void setup()
{
    Serial.begin(115200);

    pinMode(TEST_BUTTON_PIN, INPUT_PULLUP);

    relay.begin();

    printHeader();

    Serial.println("Initial relay state:");
    printRelayState();
    Serial.println();
    Serial.println("Ready. Press the GPIO25 test button.");
}

void loop()
{
    // برای پایان یافتن Pulse حتماً باید دائماً صدا زده شود.
    relay.update();

    if (wasTestButtonPressed())
    {
        runNextTestStep();
    }
}