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

    // پیام جدید، پیام خوانده‌نشده قبلی را در Buffer تک‌عضوی جایگزین می‌کند.
    IRMessage pendingMessage;

    static IRProtocol mapProtocol(decode_type_t protocol);
    bool copyRawData();

public:
    explicit IRReceiver(uint8_t gpio);

    IRReceiver(const IRReceiver&) = delete;
    IRReceiver& operator=(const IRReceiver&) = delete;

    void begin();
    void update();
    bool available() const;
    const IRMessage& peek() const;
    IRMessage read();
    void clear();
    uint8_t getPin() const;
};

#endif
