#include "Buzzer.h"

Buzzer::Buzzer(uint8_t gpio, bool activeHigh) :
    pin(gpio),
    activeHigh(activeHigh),
    state(false),
    patternActive(false),
    patternOnPhase(false),
    patternCount(0),
    completedBeeps(0),
    onDuration(0),
    offDuration(0),
    phaseStart(0),
    lastActivityMs(0)
{
}

void Buzzer::begin()
{
    pinMode(pin, OUTPUT);
    off();
}

void Buzzer::update()
{
    if (!patternActive)
        return;

    uint32_t now = millis();
    uint32_t phaseDuration = patternOnPhase ? onDuration : offDuration;

    if ((now - phaseStart) < phaseDuration)
        return;

    if (patternOnPhase)
    {
        writeState(false);
        completedBeeps++;

        if (completedBeeps >= patternCount)
        {
            patternActive = false;
            patternOnPhase = false;
            return;
        }

        patternOnPhase = false;
        phaseStart = now;
    }
    else
    {
        writeState(true);
        patternOnPhase = true;
        phaseStart = now;
    }
}

void Buzzer::on()
{
    patternActive = false;
    writeState(true);
}

void Buzzer::off()
{
    patternActive = false;
    writeState(false);
}

bool Buzzer::isOn() const
{
    return state;
}

void Buzzer::beep(uint32_t durationMs)
{
    beepPattern(1, durationMs, 0);
}

void Buzzer::beepPattern(uint8_t count, uint32_t onDurationMs, uint32_t offDurationMs)
{
    if (count == 0 || onDurationMs == 0)
    {
        stop();
        return;
    }

    // الگوی جدید، الگوی قبلی را جایگزین می‌کند.
    patternCount = count;
    completedBeeps = 0;
    onDuration = onDurationMs;
    offDuration = offDurationMs;
    patternActive = true;
    patternOnPhase = true;
    phaseStart = millis();

    writeState(true);
}

bool Buzzer::isBusy() const
{
    return patternActive;
}

bool Buzzer::isIdle() const
{
    return !patternActive;
}

uint32_t Buzzer::getLastActivityTime() const
{
    return lastActivityMs;
}

void Buzzer::stop()
{
    patternActive = false;
    patternOnPhase = false;
    writeState(false);
}

uint8_t Buzzer::getPin() const
{
    return pin;
}

void Buzzer::writeState(bool enabled)
{
    digitalWrite(pin, enabled == activeHigh ? HIGH : LOW);
    state = enabled;
    lastActivityMs = millis();
}
