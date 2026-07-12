#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

// Driver عمومی سخت‌افزار Buzzer.
// این کلاس معنی Patternها را نمی‌داند، Event تولید نمی‌کند، Learn Mode را مدیریت نمی‌کند
// و Notification ارسال نمی‌کند؛ فقط امکان اجرای Patternهای عمومی روی Buzzer را فراهم می‌کند.
class Buzzer
{
private:
    uint8_t pin;
    bool activeHigh;
    bool state;

    bool patternActive;
    bool patternOnPhase;
    uint8_t patternCount;
    uint8_t completedBeeps;
    uint32_t onDuration;
    uint32_t offDuration;
    uint32_t phaseStart;
    uint32_t lastActivityMs;

    void writeState(bool enabled);

public:
    explicit Buzzer(
        uint8_t gpio,
        bool activeHigh = true
    );

    void begin();
    void update();

    void on();
    void off();

    bool isOn() const;

    void beep(uint32_t durationMs = 100);

    void beepPattern(
        uint8_t count,
        uint32_t onDurationMs,
        uint32_t offDurationMs
    );

    bool isBusy() const;
    bool isIdle() const;

    uint32_t getLastActivityTime() const;

    void stop();

    uint8_t getPin() const;
};

#endif
