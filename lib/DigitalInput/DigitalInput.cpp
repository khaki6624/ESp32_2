#include "DigitalInput.h"

DigitalInput::DigitalInput(uint8_t gpio, bool activeLow, bool usePullup, uint32_t debounceMs)
{
    pin = gpio;
    this->activeLow = activeLow;
    this->usePullup = usePullup;

    currentState = false;
    previousState = false;
    lastRawState = false;

    lastChangeTime = 0;
    debounceDelay = debounceMs;

    changedFlag = false;
}

void DigitalInput::begin()
{
    // تنظیم حالت GPIO به عنوان ورودی
    if (usePullup)
        pinMode(pin, INPUT_PULLUP);
    else
        pinMode(pin, INPUT);

    // خواندن وضعیت اولیه GPIO
    bool raw = digitalRead(pin);

    // تبدیل وضعیت خام به وضعیت منطقی فعال/غیرفعال
    currentState = activeLow ? (raw == LOW) : (raw == HIGH);

    previousState = currentState;
    lastRawState = raw;
    changedFlag = false;
}

void DigitalInput::update()
{
    // خواندن مقدار خام از GPIO
    bool raw = digitalRead(pin);

    // اگر مقدار خام تغییر کرد، زمان تغییر ثبت می‌شود
    if (raw != lastRawState)
    {
        lastRawState = raw;
        lastChangeTime = millis();
    }

    // اگر مقدار خام به اندازه کافی پایدار مانده باشد، وضعیت منطقی به‌روزرسانی می‌شود
    if ((millis() - lastChangeTime) >= debounceDelay)
    {
        bool newState = activeLow ? (raw == LOW) : (raw == HIGH);

        if (newState != currentState)
        {
            previousState = currentState;
            currentState = newState;
            changedFlag = true;
        }
        else
        {
            changedFlag = false;
        }
    }
    else
    {
        changedFlag = false;
    }
}

bool DigitalInput::isActive()
{
    return currentState;
}

bool DigitalInput::hasChanged()
{
    return changedFlag;
}

bool DigitalInput::wasActivated()
{
    return changedFlag && currentState == true;
}

bool DigitalInput::wasDeactivated()
{
    return changedFlag && currentState == false;
}

void DigitalInput::setActiveLow(bool enable)
{
    activeLow = enable;
}

uint8_t DigitalInput::getPin()
{
    return pin;
}