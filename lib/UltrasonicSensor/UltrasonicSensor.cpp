#include "UltrasonicSensor.h"

UltrasonicSensor::UltrasonicSensor(
    uint8_t triggerPin,
    uint8_t echoPin,
    uint32_t intervalMs,
    uint32_t timeoutUs
) :
    triggerPin(triggerPin),
    echoPin(echoPin),
    interval(intervalMs),
    timeout(timeoutUs),
    lastMeasureTime(0),
    distanceCentimeters(0.0f),
    availableFlag(false),
    validFlag(false)
{
}

void UltrasonicSensor::begin()
{
    pinMode(triggerPin, OUTPUT);
    pinMode(echoPin, INPUT);

    digitalWrite(triggerPin, LOW);
    clear();
    lastMeasureTime = millis();
}

void UltrasonicSensor::update()
{
    uint32_t now = millis();

    if ((now - lastMeasureTime) < interval)
        return;

    lastMeasureTime = now;

    digitalWrite(triggerPin, LOW);
    // این مکث بسیار کوتاه فقط برای شکل‌دهی پالس سخت‌افزاری Trigger است و delay منطقی طولانی نیست.
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);

    // pulseIn ذاتاً Blocking است؛ Timeout محدود مانع گیر کردن طولانی loop می‌شود.
    uint32_t duration = pulseIn(echoPin, HIGH, timeout);

    if (duration == 0)
    {
        distanceCentimeters = 0.0f;
        validFlag = false;
        availableFlag = true;
        return;
    }

    float centimeters = (static_cast<float>(duration) * 0.0343f) / 2.0f;

    if (centimeters <= 0.0f || centimeters > 600.0f)
    {
        distanceCentimeters = 0.0f;
        validFlag = false;
    }
    else
    {
        distanceCentimeters = centimeters;
        validFlag = true;
    }

    availableFlag = true;
}

bool UltrasonicSensor::available() const
{
    return availableFlag;
}

float UltrasonicSensor::readCentimeters()
{
    float value = distanceCentimeters;
    availableFlag = false;
    return value;
}

float UltrasonicSensor::peekCentimeters() const
{
    return distanceCentimeters;
}

bool UltrasonicSensor::isValid() const
{
    return validFlag;
}

void UltrasonicSensor::clear()
{
    distanceCentimeters = 0.0f;
    availableFlag = false;
    validFlag = false;
}

uint8_t UltrasonicSensor::getTriggerPin() const
{
    return triggerPin;
}

uint8_t UltrasonicSensor::getEchoPin() const
{
    return echoPin;
}
