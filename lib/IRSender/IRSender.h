#ifndef IR_SENDER_H
#define IR_SENDER_H

#include <Arduino.h>
#include <IRsend.h>
#include <IRremoteESP8266.h>
#include <IRCommon.h>

class IRSender
{
private:
    // شماره GPIO متصل به LED فرستنده IR
    uint8_t pin;

    // شیء اصلی ارسال IR از کتابخانه IRremoteESP8266
    IRsend* sender;

public:
    // سازنده کلاس
    IRSender(uint8_t gpio);

    // آزادسازی حافظه
    ~IRSender();

    // راه‌اندازی فرستنده IR
    void begin();

    // ارسال کد با پروتکل مشخص
    bool send(const IRMessage& message);
    bool send(IRProtocol protocol, uint64_t code, uint16_t bits);

    // دریافت شماره GPIO
    uint8_t getPin() const;
};

#endif
