#include <Arduino.h>
#include <Relay.h>

Relay relay1(23);

void setup()
{
    relay1.begin();

    // روشن شدن رله به مدت 2 ثانیه
    relay1.pulse(2000);
}

void loop()
{
    relay1.update();
}