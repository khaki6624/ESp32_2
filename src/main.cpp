#include <Arduino.h>
#include <BoardConfig.h>
#include <DeviceIdentity.h>

void setup()
{
    Serial.begin(115200);

    Serial.println(DeviceIdentity::DEVICE_NAME);

    Serial.println(BoardConfig::OUTPUT_COUNT);

    Serial.println(BoardConfig::HAS_WIFI);

    Serial.println(BoardConfig::outputs[0].gpio);
}

void loop()
{

}
