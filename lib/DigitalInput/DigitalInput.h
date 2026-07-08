#ifndef DIGITAL_INPUT_H
#define DIGITAL_INPUT_H

#include <Arduino.h>

class DigitalInput
{
private:
    // شماره GPIO ورودی
    uint8_t pin;

    // اگر true باشد، LOW یعنی فعال
    bool activeLow;

    // آیا از Pullup داخلی ESP32 استفاده شود؟
    bool usePullup;

    // وضعیت پایدار فعلی ورودی
    bool currentState;

    // وضعیت پایدار قبلی ورودی
    bool previousState;

    // مقدار خام قبلی خوانده‌شده از GPIO
    bool lastRawState;

    // آخرین زمانی که مقدار خام تغییر کرد
    uint32_t lastChangeTime;

    // زمان لازم برای حذف نویز کلید یا سنسور
    uint32_t debounceDelay;

    // آیا از آخرین update تغییر پایدار رخ داده است؟
    bool changedFlag;

public:
    // سازنده کلاس
    DigitalInput(uint8_t gpio, bool activeLow = true, bool usePullup = true, uint32_t debounceMs = 50);

    // راه‌اندازی GPIO
    void begin();

    // بررسی ورودی بدون delay
    void update();

    // وضعیت فعلی ورودی
    bool isActive();

    // آیا وضعیت تغییر کرده است؟
    bool hasChanged();

    // آیا همین الان فعال شده است؟
    bool wasActivated();

    // آیا همین الان غیرفعال شده است؟
    bool wasDeactivated();

    // تغییر تنظیم Active Low
    void setActiveLow(bool enable);

    // دریافت شماره GPIO
    uint8_t getPin();
};

#endif