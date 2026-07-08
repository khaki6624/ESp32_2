#include "AnalogInput.h"

AnalogInput::AnalogInput(
    uint8_t gpio,
    float referenceVoltage,
    uint16_t adcMax,
    float filterAlpha,
    uint16_t changeThreshold
)
{
    pin = gpio;

    rawValue = 0;
    previousRawValue = 0;
    filteredValue = 0;

    this->referenceVoltage = referenceVoltage;
    this->adcMax = adcMax;
    this->filterAlpha = filterAlpha;
    this->changeThreshold = changeThreshold;

    scaleMin = 0;
    scaleMax = 100;

    changedFlag = false;
}

void AnalogInput::begin()
{
    // تنظیم پایه به‌عنوان ورودی
    pinMode(pin, INPUT);

    // خواندن مقدار اولیه ADC
    rawValue = analogRead(pin);

    previousRawValue = rawValue;

    // مقدار اولیه فیلتر برابر مقدار خام اولیه قرار داده می‌شود
    filteredValue = rawValue;

    changedFlag = false;
}

void AnalogInput::update()
{
    // خواندن مقدار خام جدید از ADC
    rawValue = analogRead(pin);

    // محاسبه تغییر معنادار بر اساس threshold
    if (abs((int)rawValue - (int)previousRawValue) >= changeThreshold)
        changedFlag = true;
    else
        changedFlag = false;

    // اعمال فیلتر نرم نمایی
    filteredValue = (filterAlpha * rawValue) + ((1.0 - filterAlpha) * filteredValue);

    // ذخیره مقدار فعلی برای مقایسه بعدی
    previousRawValue = rawValue;
}

uint16_t AnalogInput::getRaw()
{
    return rawValue;
}

float AnalogInput::getFiltered()
{
    return filteredValue;
}

float AnalogInput::getVoltage()
{
    // تبدیل مقدار ADC به ولتاژ
    return (filteredValue / adcMax) * referenceVoltage;
}

float AnalogInput::getScaled()
{
    // تبدیل مقدار ADC به بازه دلخواه
    float normalized = filteredValue / adcMax;

    return scaleMin + (normalized * (scaleMax - scaleMin));
}

void AnalogInput::setScale(float minValue, float maxValue)
{
    scaleMin = minValue;
    scaleMax = maxValue;
}

void AnalogInput::setFilterAlpha(float alpha)
{
    // محدود کردن alpha بین 0 و 1
    if (alpha < 0)
        alpha = 0;

    if (alpha > 1)
        alpha = 1;

    filterAlpha = alpha;
}

void AnalogInput::setChangeThreshold(uint16_t threshold)
{
    changeThreshold = threshold;
}

bool AnalogInput::hasChanged()
{
    return changedFlag;
}

uint8_t AnalogInput::getPin()
{
    return pin;
}