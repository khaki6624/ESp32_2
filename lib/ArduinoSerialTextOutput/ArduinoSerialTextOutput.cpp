#include "ArduinoSerialTextOutput.h"

ArduinoSerialTextOutput::ArduinoSerialTextOutput(Print& output) : output_(output) {}

TextOutputResult ArduinoSerialTextOutput::write(const char* data, size_t length)
{
    if (data == nullptr || length == 0U) return TextOutputResult::FAILED;

    const size_t written = output_.write(
        reinterpret_cast<const uint8_t*>(data), length);
    if (written == length) return TextOutputResult::SUCCESS;
    if (written == 0U) return TextOutputResult::RETRY_LATER;
    return TextOutputResult::FAILED;
}
