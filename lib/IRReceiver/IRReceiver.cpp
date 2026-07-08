#include "IRReceiver.h"

IRReceiver::IRReceiver(uint8_t gpio)
{
    pin = gpio;

    // ساخت شیء گیرنده IR روی پایه مشخص‌شده
    receiver = new IRrecv(pin);

    availableFlag = false;
    lastCode = 0;
    lastBits = 0;
    lastProtocol = "UNKNOWN";
}

IRReceiver::~IRReceiver()
{
    // آزاد کردن حافظه اختصاص‌داده‌شده به گیرنده
    if (receiver != nullptr)
    {
        delete receiver;
        receiver = nullptr;
    }
}

void IRReceiver::begin()
{
    // فعال‌سازی گیرنده IR
    receiver->enableIRIn();

    availableFlag = false;
}

void IRReceiver::update()
{
    // بررسی غیرمسدودکننده برای دریافت سیگنال جدید
    if (receiver->decode(&results))
    {
        // ذخیره مقدار کد دریافتی
        lastCode = results.value;

        // ذخیره تعداد بیت‌های کد
        lastBits = results.bits;

        // ذخیره نام پروتکل
        lastProtocol = typeToString(results.decode_type);

        // اعلام آماده بودن کد جدید
        availableFlag = true;

        // آماده‌سازی گیرنده برای دریافت سیگنال بعدی
        receiver->resume();
    }
}

bool IRReceiver::available()
{
    return availableFlag;
}

uint64_t IRReceiver::readCode()
{
    // بعد از خواندن، وضعیت available پاک می‌شود
    availableFlag = false;

    return lastCode;
}

uint16_t IRReceiver::getBits()
{
    return lastBits;
}

String IRReceiver::getProtocol()
{
    return lastProtocol;
}

void IRReceiver::clear()
{
    availableFlag = false;
    lastCode = 0;
    lastBits = 0;
    lastProtocol = "UNKNOWN";
}

uint8_t IRReceiver::getPin()
{
    return pin;
}