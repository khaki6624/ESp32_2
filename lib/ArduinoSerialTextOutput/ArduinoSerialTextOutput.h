#ifndef ARDUINO_SERIAL_TEXT_OUTPUT_H
#define ARDUINO_SERIAL_TEXT_OUTPUT_H

#include <Print.h>
#include <TextOutput.h>

class ArduinoSerialTextOutput final : public TextOutput
{
public:
    explicit ArduinoSerialTextOutput(Print& output);
    TextOutputResult write(const char* data, size_t length) override;

private:
    Print& output_;
};

#endif
