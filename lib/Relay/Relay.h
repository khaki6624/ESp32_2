#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>

class Relay
{
private:
    uint8_t pin;
    bool state;
    bool activeLow;

    bool pulseActive;
    uint32_t pulseStart;
    uint32_t pulseDuration;

public:
    Relay(uint8_t gpio, bool activeLow = false);

    void begin();
    void update();

    void on();
    void off();
    void toggle();

    void setState(bool state);
    bool isOn();

    void setActiveLow(bool enable);

    void pulse(uint32_t duration);

    uint8_t getPin();
};

#endif