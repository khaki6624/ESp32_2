#include "DelsamApplication.h"

#include <Arduino.h>
#include <DelsamCompositionRoot.h>

void DelsamApplication::begin(uint32_t nowMs)
{
    Serial.begin(115200);
    DelsamCompositionRoot::begin(nowMs);
}

void DelsamApplication::update(uint32_t nowMs)
{
    DelsamCompositionRoot::update(nowMs);
}
