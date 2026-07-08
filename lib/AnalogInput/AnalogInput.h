#ifndef ANALOG_INPUT_H
#define ANALOG_INPUT_H

#include <Arduino.h>

class AnalogInput
{
private:
    // شماره GPIO ورودی آنالوگ
    uint8_t pin;

    // مقدار خام فعلی ADC
    uint16_t rawValue;

    // مقدار خام قبلی ADC
    uint16_t previousRawValue;

    // مقدار فیلترشده
    float filteredValue;

    // ضریب فیلتر نرم؛ عدد بین 0 تا 1
    float filterAlpha;

    // ولتاژ مرجع ADC
    float referenceVoltage;

    // بیشترین مقدار ADC
    uint16_t adcMax;

    // حداقل و حداکثر بازه خروجی مقیاس‌شده
    float scaleMin;
    float scaleMax;

    // آستانه تشخیص تغییر
    uint16_t changeThreshold;

    // آیا مقدار از آخرین update تغییر معنادار داشته است؟
    bool changedFlag;

public:
    // سازنده کلاس
    AnalogInput(
        uint8_t gpio,
        float referenceVoltage = 3.3,
        uint16_t adcMax = 4095,
        float filterAlpha = 0.2,
        uint16_t changeThreshold = 10
    );

    // آماده‌سازی ورودی
    void begin();

    // خواندن و به‌روزرسانی مقدار ورودی
    void update();

    // مقدار خام ADC
    uint16_t getRaw();

    // مقدار فیلترشده
    float getFiltered();

    // ولتاژ محاسبه‌شده
    float getVoltage();

    // مقدار مقیاس‌شده، مثلاً 0 تا 100 درصد
    float getScaled();

    // تنظیم بازه خروجی مقیاس‌شده
    void setScale(float minValue, float maxValue);

    // تنظیم ضریب فیلتر
    void setFilterAlpha(float alpha);

    // تنظیم آستانه تغییر
    void setChangeThreshold(uint16_t threshold);

    // آیا تغییر معنادار رخ داده است؟
    bool hasChanged();

    // دریافت شماره GPIO
    uint8_t getPin();
};

#endif