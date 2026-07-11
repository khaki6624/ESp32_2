#ifndef RF_RECEIVER_H
#define RF_RECEIVER_H

#include <Arduino.h>
#include <RCSwitch.h>
#include <RFCommon.h>

class RFReceiver
{
private:
    uint8_t pin;
    RCSwitch receiver;

    // آخرین پیام RF خوانده‌نشده
    RFMessage lastMessage;

    RFProtocol mapProtocol(uint8_t protocol) const;

public:
    explicit RFReceiver(uint8_t gpio);

    void begin();
    void update();

    bool available() const;

    // مشاهده پیام بدون پاک کردن آن
    const RFMessage& peek() const;

    // خواندن پیام و پاک کردن آن از Driver
    RFMessage read();

    void clear();

    uint8_t getPin() const;
};

#endif
