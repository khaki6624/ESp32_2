#ifndef IR_RECEIVER_H
#define IR_RECEIVER_H

#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>

class IRReceiver
{
private:
    // شماره GPIO متصل به خروجی گیرنده IR مثل VS1838B
    uint8_t pin;

    // شیء اصلی کتابخانه IRremoteESP8266
    IRrecv* receiver;

    // نتیجه Decode شده آخرین سیگنال دریافتی
    decode_results results;

    // آیا کد جدید آماده خواندن است؟
    bool availableFlag;

    // مقدار عددی آخرین کد دریافتی
    uint64_t lastCode;

    // تعداد بیت‌های کد دریافتی
    uint16_t lastBits;

    // نام پروتکل تشخیص داده‌شده
    String lastProtocol;

public:
    // سازنده کلاس
    IRReceiver(uint8_t gpio);

    // آزادسازی حافظه
    ~IRReceiver();

    // راه‌اندازی گیرنده IR
    void begin();

    // بررسی دریافت کد جدید، بدون delay
    void update();

    // آیا کد جدید دریافت شده است؟
    bool available();

    // خواندن آخرین کد دریافتی
    uint64_t readCode();

    // گرفتن تعداد بیت‌های آخرین کد
    uint16_t getBits();

    // گرفتن نام پروتکل آخرین کد
    String getProtocol();

    // پاک کردن وضعیت کد آماده
    void clear();

    // دریافت شماره GPIO
    uint8_t getPin();
};

#endif