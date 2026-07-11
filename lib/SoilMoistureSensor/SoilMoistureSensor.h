#ifndef SOIL_MOISTURE_SENSOR_H
#define SOIL_MOISTURE_SENSOR_H

#include <Arduino.h>
#include <AnalogInput.h>

class SoilMoistureSensor
{
private:
    AnalogInput& analogInput;
    uint16_t dryValue;
    uint16_t wetValue;
    float dryThreshold;
    float wetThreshold;
    float percent;
    uint16_t rawValue;
    bool validFlag;

    void calculatePercent();
    float clampPercent(float value) const;

public:
    SoilMoistureSensor(
        AnalogInput& analogInput,
        uint16_t dryValue,
        uint16_t wetValue,
        float dryThresholdPercent = 30.0f,
        float wetThresholdPercent = 70.0f
    );

    void begin();
    void update();

    float getPercent() const;
    uint16_t getRawValue() const;

    bool isDry() const;
    bool isWet() const;
    bool isValid() const;

    void setCalibration(
        uint16_t dryValue,
        uint16_t wetValue
    );

    void setThresholds(
        float dryThresholdPercent,
        float wetThresholdPercent
    );
};

#endif
