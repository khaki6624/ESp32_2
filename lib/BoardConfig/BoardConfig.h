#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <Arduino.h>

namespace BoardConfig
{

//==================================================
// تنظیمات IR
//==================================================

constexpr bool HAS_IR_RECEIVER = true;
constexpr bool HAS_IR_SENDER   = true;

constexpr uint8_t IR_RECEIVER_PIN = 4;
constexpr uint8_t IR_SENDER_PIN   = 5;

    //==================================================
    // ساختار تنظیمات خروجی‌ها
    //==================================================

    struct OutputConfig
    {
        uint8_t gpio;      // شماره GPIO خروجی
        bool activeLow;    // اگر true باشد، خروجی با LOW فعال می‌شود
    };

    //==================================================
    // ساختار تنظیمات ورودی‌های دیجیتال
    //==================================================

    struct InputConfig
    {
        uint8_t gpio;          // شماره GPIO ورودی
        bool activeLow;        // اگر true باشد، LOW یعنی ورودی فعال است
        bool usePullup;        // استفاده از Pullup داخلی ESP32
        uint32_t debounceMs;   // زمان حذف نویز ورودی
    };

    //==================================================
    // ساختار تنظیمات ورودی‌های آنالوگ
    //==================================================

    struct AnalogInputConfig
    {
        uint8_t gpio;              // شماره GPIO آنالوگ
        float referenceVoltage;    // ولتاژ مرجع ADC
        uint16_t adcMax;           // بیشترین مقدار ADC
        float filterAlpha;         // ضریب فیلتر نرم
        uint16_t threshold;        // آستانه تشخیص تغییر
        float scaleMin;            // حداقل مقدار مقیاس‌شده
        float scaleMax;            // حداکثر مقدار مقیاس‌شده
    };

    //==================================================
    // قابلیت‌های سخت‌افزاری برد
    //==================================================

    constexpr bool HAS_WIFI  = true;
    constexpr bool HAS_RS485 = true;
    constexpr bool HAS_RF    = true;
    constexpr bool HAS_IR    = true;

    //==================================================
    // خروجی‌ها
    //==================================================

    constexpr uint8_t OUTPUT_COUNT = 16;

    constexpr OutputConfig outputs[OUTPUT_COUNT] =
    {
        {23, true},
        {22, true},
        {21, true},
        {19, true},
        {18, true},
        {17, true},
        {16, true},
        {15, true},

        {14, true},
        {13, true},
        {12, true},
        {27, true},
        {26, true},
        {25, true},
        {33, true},
        {32, true}
    };

    //==================================================
    // ورودی‌های دیجیتال
    //==================================================

    constexpr uint8_t INPUT_COUNT = 1;

    constexpr InputConfig inputs[INPUT_COUNT] =
    {
        // gpio, activeLow, usePullup, debounceMs
        {34, true, false, 50}
    };

    //==================================================
    // ورودی‌های آنالوگ
    //==================================================

    constexpr uint8_t ANALOG_INPUT_COUNT = 1;

    constexpr AnalogInputConfig analogInputs[ANALOG_INPUT_COUNT] =
    {
        // gpio, referenceVoltage, adcMax, filterAlpha, threshold, scaleMin, scaleMax
        {35, 3.3, 4095, 0.2, 10, 0, 100}
    };
}

#endif
