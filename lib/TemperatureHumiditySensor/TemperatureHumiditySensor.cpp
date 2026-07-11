#include "TemperatureHumiditySensor.h"

MockTemperatureHumiditySensor::MockTemperatureHumiditySensor() :
    reading{},
    availableFlag(false)
{
}

void MockTemperatureHumiditySensor::begin()
{
    clear();
}

void MockTemperatureHumiditySensor::update()
{
    // Mock سخت‌افزار واقعی ندارد؛ مقدارها با setMockValues تزریق می‌شوند.
}

bool MockTemperatureHumiditySensor::available() const
{
    return availableFlag;
}

TemperatureHumidityReading MockTemperatureHumiditySensor::read()
{
    TemperatureHumidityReading value = reading;
    availableFlag = false;
    return value;
}

const TemperatureHumidityReading& MockTemperatureHumiditySensor::peek() const
{
    return reading;
}

void MockTemperatureHumiditySensor::clear()
{
    reading = TemperatureHumidityReading{};
    availableFlag = false;
}

void MockTemperatureHumiditySensor::setMockValues(
    float temperatureCelsius,
    float humidityPercent,
    bool valid
)
{
    reading.temperatureCelsius = temperatureCelsius;
    reading.humidityPercent = humidityPercent;
    reading.valid = valid;
    reading.timestampMs = millis();
    availableFlag = true;
}
