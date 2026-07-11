#include <Arduino.h>
#include <BoardConfig.h>
#include <DeviceIdentity.h>
#include <RFCommon.h>

//==================================================
// فعال‌سازی تست Driverها
//==================================================

#define TEST_RF_RECEIVER 0
#define TEST_RF_SENDER 0
#define TEST_BUZZER 0
#define TEST_ULTRASONIC 0
#define TEST_SOIL_MOISTURE 0
#define TEST_TEMPERATURE_HUMIDITY_MOCK 0

#if TEST_RF_RECEIVER
#include <RFReceiver.h>
RFReceiver rfReceiver(BoardConfig::RF_RECEIVER_PIN);
#endif

#if TEST_RF_SENDER
#include <RFSender.h>
RFSender rfSender(BoardConfig::RF_SENDER_PIN);
uint32_t lastRfSendMs = 0;
#endif

#if TEST_BUZZER
#include <Buzzer.h>
Buzzer buzzer(BoardConfig::BUZZER_PIN);
uint32_t lastBuzzerPatternMs = 0;
#endif

#if TEST_ULTRASONIC
#include <UltrasonicSensor.h>
UltrasonicSensor ultrasonicSensor(
    BoardConfig::ULTRASONIC_TRIGGER_PIN,
    BoardConfig::ULTRASONIC_ECHO_PIN
);
#endif

#if TEST_SOIL_MOISTURE
#include <AnalogInput.h>
#include <SoilMoistureSensor.h>
constexpr BoardConfig::AnalogInputConfig soilAnalogConfig =
    BoardConfig::analogInputs[BoardConfig::SOIL_MOISTURE_ADC_INDEX];
AnalogInput soilAnalogInput(
    soilAnalogConfig.gpio,
    soilAnalogConfig.referenceVoltage,
    soilAnalogConfig.adcMax,
    soilAnalogConfig.filterAlpha,
    soilAnalogConfig.threshold
);
SoilMoistureSensor soilMoistureSensor(soilAnalogInput, 3000, 1200);
uint32_t lastSoilPrintMs = 0;
#endif

#if TEST_TEMPERATURE_HUMIDITY_MOCK
#include <TemperatureHumiditySensor.h>
MockTemperatureHumiditySensor temperatureHumiditySensor;
uint32_t lastTemperatureMockMs = 0;
#endif

bool isValidPin(uint8_t pin)
{
    return pin != BoardConfig::INVALID_PIN;
}

const char* rfProtocolName(RFProtocol protocol)
{
    switch (protocol)
    {
        case RFProtocol::PROTOCOL_1:  return "PROTOCOL_1";
        case RFProtocol::PROTOCOL_2:  return "PROTOCOL_2";
        case RFProtocol::PROTOCOL_3:  return "PROTOCOL_3";
        case RFProtocol::PROTOCOL_4:  return "PROTOCOL_4";
        case RFProtocol::PROTOCOL_5:  return "PROTOCOL_5";
        case RFProtocol::PROTOCOL_6:  return "PROTOCOL_6";
        case RFProtocol::PROTOCOL_7:  return "PROTOCOL_7";
        case RFProtocol::PROTOCOL_8:  return "PROTOCOL_8";
        case RFProtocol::PROTOCOL_9:  return "PROTOCOL_9";
        case RFProtocol::PROTOCOL_10: return "PROTOCOL_10";
        case RFProtocol::PROTOCOL_11: return "PROTOCOL_11";
        case RFProtocol::PROTOCOL_12: return "PROTOCOL_12";
        default:                      return "UNKNOWN";
    }
}

void printUint64Hex(uint64_t value)
{
    uint32_t high = static_cast<uint32_t>(value >> 32);
    uint32_t low = static_cast<uint32_t>(value & 0xFFFFFFFFULL);

    if (high > 0)
        Serial.print(high, HEX);

    Serial.print(low, HEX);
}

void setup()
{
    Serial.begin(115200);

    Serial.println(DeviceIdentity::DEVICE_NAME);
    Serial.println("Driver MVP test harness");

#if TEST_RF_RECEIVER
    if (isValidPin(BoardConfig::RF_RECEIVER_PIN))
        rfReceiver.begin();
#endif

#if TEST_RF_SENDER
    if (isValidPin(BoardConfig::RF_SENDER_PIN))
        rfSender.begin();
#endif

#if TEST_BUZZER
    if (isValidPin(BoardConfig::BUZZER_PIN))
        buzzer.begin();
#endif

#if TEST_ULTRASONIC
    if (
        isValidPin(BoardConfig::ULTRASONIC_TRIGGER_PIN) &&
        isValidPin(BoardConfig::ULTRASONIC_ECHO_PIN)
    )
        ultrasonicSensor.begin();
#endif

#if TEST_SOIL_MOISTURE
    soilMoistureSensor.begin();
#endif

#if TEST_TEMPERATURE_HUMIDITY_MOCK
    temperatureHumiditySensor.begin();
    temperatureHumiditySensor.setMockValues(24.5f, 48.0f);
#endif
}

void loop()
{
    uint32_t now = millis();

#if TEST_RF_RECEIVER
    if (isValidPin(BoardConfig::RF_RECEIVER_PIN))
    {
        rfReceiver.update();

        if (rfReceiver.available())
        {
            RFMessage message = rfReceiver.read();

            Serial.print("RF Protocol: ");
            Serial.println(rfProtocolName(message.protocol));
            Serial.print("RF Code: 0x");
            printUint64Hex(message.code);
            Serial.println();
            Serial.print("RF Bits: ");
            Serial.println(message.bits);
            Serial.print("RF Pulse: ");
            Serial.println(message.pulseLength);
            Serial.print("RF Valid: ");
            Serial.println(message.valid ? "true" : "false");
        }
    }
#endif

#if TEST_RF_SENDER
    if (isValidPin(BoardConfig::RF_SENDER_PIN) && (now - lastRfSendMs >= 5000))
    {
        lastRfSendMs = now;

        RFMessage sampleMessage;
        sampleMessage.protocol = RFProtocol::PROTOCOL_1;
        sampleMessage.code = 0x123456;
        sampleMessage.bits = 24;
        sampleMessage.valid = true;

        bool sent = rfSender.send(sampleMessage);
        Serial.print("RF sample sent: ");
        Serial.println(sent ? "true" : "false");
    }
#endif

#if TEST_BUZZER
    if (isValidPin(BoardConfig::BUZZER_PIN))
    {
        buzzer.update();

        if (!buzzer.isBusy() && (now - lastBuzzerPatternMs >= 3000))
        {
            lastBuzzerPatternMs = now;
            buzzer.beepPattern(2, 100, 100);
        }
    }
#endif

#if TEST_ULTRASONIC
    if (
        isValidPin(BoardConfig::ULTRASONIC_TRIGGER_PIN) &&
        isValidPin(BoardConfig::ULTRASONIC_ECHO_PIN)
    )
    {
        ultrasonicSensor.update();

        if (ultrasonicSensor.available())
        {
            float distance = ultrasonicSensor.readCentimeters();

            Serial.print("Ultrasonic cm: ");
            Serial.print(distance);
            Serial.print(" valid: ");
            Serial.println(ultrasonicSensor.isValid() ? "true" : "false");
        }
    }
#endif

#if TEST_SOIL_MOISTURE
    soilMoistureSensor.update();

    if (now - lastSoilPrintMs >= 1000)
    {
        lastSoilPrintMs = now;

        Serial.print("Soil raw: ");
        Serial.print(soilMoistureSensor.getRawValue());
        Serial.print(" percent: ");
        Serial.print(soilMoistureSensor.getPercent());
        Serial.print(" dry: ");
        Serial.print(soilMoistureSensor.isDry() ? "true" : "false");
        Serial.print(" wet: ");
        Serial.println(soilMoistureSensor.isWet() ? "true" : "false");
    }
#endif

#if TEST_TEMPERATURE_HUMIDITY_MOCK
    temperatureHumiditySensor.update();

    if (now - lastTemperatureMockMs >= 2000)
    {
        lastTemperatureMockMs = now;
        temperatureHumiditySensor.setMockValues(24.5f, 48.0f);
    }

    if (temperatureHumiditySensor.available())
    {
        TemperatureHumidityReading reading = temperatureHumiditySensor.read();

        Serial.print("Mock temperature C: ");
        Serial.print(reading.temperatureCelsius);
        Serial.print(" humidity %: ");
        Serial.print(reading.humidityPercent);
        Serial.print(" valid: ");
        Serial.println(reading.valid ? "true" : "false");
    }
#endif
}
