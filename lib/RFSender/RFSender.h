#ifndef RF_SENDER_H
#define RF_SENDER_H

#include <Arduino.h>
#include <RCSwitch.h>
#include <RFCommon.h>

class RFSender
{
private:
    uint8_t pin;
    RCSwitch sender;

    uint8_t mapProtocol(RFProtocol protocol) const;

public:
    explicit RFSender(uint8_t gpio);

    void begin();

    bool send(const RFMessage& message);

    bool send(
        RFProtocol protocol,
        uint64_t code,
        uint16_t bits,
        uint16_t pulseLength = 0,
        uint8_t repeatCount = 0
    );

    uint8_t getPin() const;
};

#endif
