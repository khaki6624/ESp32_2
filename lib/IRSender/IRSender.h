#ifndef IR_SENDER_H
#define IR_SENDER_H

#include <Arduino.h>
#include <IRsend.h>
#include <IRremoteESP8266.h>

enum class IRProtocol
{
    UNKNOWN = 0,
    NEC,
    SONY,
    SAMSUNG,
    LG
};

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
    bool send(IRProtocol protocol, uint64_t code, uint16_t bits);

    // ارسال کد NEC
    bool sendNEC(uint64_t code, uint16_t bits = 32);

    // ارسال کد Sony
    bool sendSony(uint64_t code, uint16_t bits = 12);

    // ارسال کد Samsung
    bool sendSamsung(uint64_t code, uint16_t bits = 32);

    // ارسال کد LG
    bool sendLG(uint64_t code, uint16_t bits = 28);

    // دریافت شماره GPIO
    uint8_t getPin();
};

#endif