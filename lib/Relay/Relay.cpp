#include "Relay.h"

Relay::Relay(uint8_t gpio, bool activeLow)
{
    pin = gpio;
    this->activeLow = activeLow;

    state = false;

    pulseActive = false;
    pulseStart = 0;
    pulseDuration = 0;
}

void Relay::begin()
{
    pinMode(pin, OUTPUT);
    off();
}

void Relay::update()
{
    if (pulseActive)
    {
        if (millis() - pulseStart >= pulseDuration)
        {
            off();
            pulseActive = false;
        }
    }
}

void Relay::on()
{
    digitalWrite(pin, activeLow ? LOW : HIGH);
    state = true;
}

void Relay::off()
{
    digitalWrite(pin, activeLow ? HIGH : LOW);
    state = false;
}

void Relay::toggle()
{
    setState(!state);
}

void Relay::setState(bool newState)
{
    if (newState)
        on();
    else
        off();
}

bool Relay::isOn()
{
    return state;
}

void Relay::setActiveLow(bool enable)
{
    activeLow = enable;
}

void Relay::pulse(uint32_t duration)
{
    on();

    pulseDuration = duration;
    pulseStart = millis();

    pulseActive = true;
}

uint8_t Relay::getPin()
{
    return pin;
}