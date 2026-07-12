#ifndef IR_SENDER_H
#define IR_SENDER_H

#include <Arduino.h>
#include <IRsend.h>
#include <IRCommon.h>

// این کلاس فقط Driver ارسال IR است و هیچ قابلیت Receive ندارد.
// این Driver هیچ Queue، Retry، Storage، Event یا Notification را مدیریت نمی‌کند.
// begin() باید قبل از اولین send() فراخوانی شود.
class IRSender
{
private:
    uint8_t pin;
    IRsend sender;
    bool initialized = false;

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
