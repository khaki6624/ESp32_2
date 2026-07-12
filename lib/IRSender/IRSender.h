#ifndef IR_SENDER_H
#define IR_SENDER_H

#include <Arduino.h>
#include <IRsend.h>
#include <IRCommon.h>

// Driver فرستنده IR؛ این کلاس فقط پیام‌های IR را ارسال می‌کند.
class IRSender
{
private:
    uint8_t pin;
    IRsend sender;

    bool sendProtocol(
        IRProtocol protocol,
        uint64_t code,
        uint16_t bits
    );

public:
    explicit IRSender(uint8_t gpio);

    void begin();

    bool send(const IRMessage& message);

    bool send(
        IRProtocol protocol,
        uint64_t code,
        uint16_t bits
    );

    uint8_t getPin() const;
};

#endif
