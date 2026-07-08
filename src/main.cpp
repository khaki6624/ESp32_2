#include <Arduino.h>
#include <BoardConfig.h>

void setup()
{
    Serial.begin(115200);

    Serial.println(BoardConfig::BOARD_NAME);

    Serial.println(BoardConfig::OUTPUT_COUNT);

    Serial.println(BoardConfig::HAS_WIFI);

    Serial.println(BoardConfig::outputs[0].gpio);
}

void loop()
{

}