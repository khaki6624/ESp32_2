#ifndef SOIL_MOISTURE_SENSOR_H
#define SOIL_MOISTURE_SENSOR_H

#include <Arduino.h>
#include <AnalogInput.h>

// SoilMoistureSensor مالک AnalogInput نیست.
// begin() و update() مربوط به AnalogInput باید توسط مالک بیرونی، مانند InputManager، مدیریت شوند.
// این کلاس فقط مقدار خام فعلی AnalogInput را به درصد رطوبت خاک تبدیل می‌کند.
// این کلاس آبیاری، Rule، Event یا Storage را مدیریت نمی‌کند.
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
    uint32_t lastUpdateTimeMs;

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
    uint32_t getLastUpdateTime() const;

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
