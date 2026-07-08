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
    IRrecv* receiver;
    decode_results results;

    // فقط پیام خوانده‌نشده در RAM نگه‌داری می‌شود.
    IRMessage pendingMessage;

    IRProtocol mapProtocol(decode_type_t protocol) const;

public:
    explicit IRReceiver(uint8_t gpio);
    ~IRReceiver();

    IRReceiver(const IRReceiver&) = delete;
    IRReceiver& operator=(const IRReceiver&) = delete;

    void begin();
    void update();
    bool available() const;
    IRMessage read();
    void clear();
    uint8_t getPin() const;
};

#endif
