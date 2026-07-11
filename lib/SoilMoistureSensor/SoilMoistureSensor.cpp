#include "SoilMoistureSensor.h"

SoilMoistureSensor::SoilMoistureSensor(
    AnalogInput& analogInput,
    uint16_t dryValue,
    uint16_t wetValue,
    float dryThresholdPercent,
    float wetThresholdPercent
) :
    analogInput(analogInput),
    dryValue(dryValue),
    wetValue(wetValue),
    dryThreshold(dryThresholdPercent),
    wetThreshold(wetThresholdPercent),
    percent(0.0f),
    rawValue(0),
    validFlag(false)
{
}

void SoilMoistureSensor::begin()
{
    // AnalogInput مالک خواندن ADC است؛ این Driver فقط مقدار آن را تفسیر می‌کند.
    analogInput.begin();
    update();
}

void SoilMoistureSensor::update()
{
    analogInput.update();
    rawValue = analogInput.getRaw();
    calculatePercent();
}

float SoilMoistureSensor::getPercent() const
{
    return percent;
}

uint16_t SoilMoistureSensor::getRawValue() const
{
    return rawValue;
}

bool SoilMoistureSensor::isDry() const
{
    return validFlag && percent <= dryThreshold;
}

bool SoilMoistureSensor::isWet() const
{
    return validFlag && percent >= wetThreshold;
}

bool SoilMoistureSensor::isValid() const
{
    return validFlag;
}

void SoilMoistureSensor::setCalibration(uint16_t newDryValue, uint16_t newWetValue)
{
    dryValue = newDryValue;
    wetValue = newWetValue;
    calculatePercent();
}

void SoilMoistureSensor::setThresholds(float dryThresholdPercent, float wetThresholdPercent)
{
    dryThreshold = clampPercent(dryThresholdPercent);
    wetThreshold = clampPercent(wetThresholdPercent);
}

void SoilMoistureSensor::calculatePercent()
{
    if (dryValue == wetValue)
    {
        percent = 0.0f;
        validFlag = false;
        return;
    }

    float span = static_cast<float>(wetValue) - static_cast<float>(dryValue);
    float value = ((static_cast<float>(rawValue) - static_cast<float>(dryValue)) * 100.0f) / span;

    // اگر مقدار wet از dry کوچک‌تر باشد، فرمول بالا همچنان جهت صحیح ۰ تا ۱۰۰ را حفظ می‌کند.
    percent = clampPercent(value);
    validFlag = true;
}

float SoilMoistureSensor::clampPercent(float value) const
{
    if (value < 0.0f)
        return 0.0f;

    if (value > 100.0f)
        return 100.0f;

    return value;
}
