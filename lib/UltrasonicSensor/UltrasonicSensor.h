#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <Arduino.h>

class UltrasonicSensor
{
private:
    uint8_t triggerPin;
    uint8_t echoPin;
    uint32_t interval;
    uint32_t timeout;
    uint32_t lastMeasureTime;
    float distanceCentimeters;
    bool availableFlag;
    bool validFlag;

public:
    UltrasonicSensor(
        uint8_t triggerPin,
        uint8_t echoPin,
        uint32_t intervalMs = 500,
        uint32_t timeoutUs = 30000
    );

    void begin();
    void update();

    bool available() const;

    float readCentimeters();
    float peekCentimeters() const;

    bool isValid() const;

    void clear();

    uint8_t getTriggerPin() const;
    uint8_t getEchoPin() const;
};

#endif
