#ifndef IR_RECEIVER_H
#define IR_RECEIVER_H

#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRCommon.h>

class IRReceiver
{
private:
    uint8_t pin;
    IRrecv receiver;
    decode_results results;

    // آخرین پیام IR خوانده‌نشده
    IRMessage lastMessage;

    IRProtocol mapProtocol(decode_type_t protocol) const;

public:
    explicit IRReceiver(uint8_t gpio);

    void begin();
    void update();

    bool available() const;

    // مشاهده پیام بدون پاک کردن آن
    const IRMessage& peek() const;

    // خواندن پیام و پاک کردن آن از Driver
    IRMessage read();

    void clear();

    uint8_t getPin() const;
};

#endif