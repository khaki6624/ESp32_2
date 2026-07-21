#include <Arduino.h>
#include <DelsamApplication.h>

void setup()
{
    DelsamApplication::begin(millis());
}

void loop()
{
    DelsamApplication::update(millis());
}
