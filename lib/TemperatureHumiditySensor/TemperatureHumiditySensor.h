#ifndef TEMPERATURE_HUMIDITY_SENSOR_H
#define TEMPERATURE_HUMIDITY_SENSOR_H

#include <Arduino.h>

struct TemperatureHumidityReading
{
    float temperatureCelsius = 0.0f;
    float humidityPercent = 0.0f;
    bool valid = false;
    uint32_t timestampMs = 0;
};

class ITemperatureHumiditySensor
{
public:
    virtual ~ITemperatureHumiditySensor() = default;

    virtual void begin() = 0;
    virtual void update() = 0;

    virtual bool available() const = 0;

    virtual TemperatureHumidityReading read() = 0;
    virtual const TemperatureHumidityReading& peek() const = 0;

    virtual void clear() = 0;
};

class MockTemperatureHumiditySensor : public ITemperatureHumiditySensor
{
private:
    TemperatureHumidityReading reading;
    bool availableFlag;

public:
    MockTemperatureHumiditySensor();

    void begin() override;
    void update() override;

    bool available() const override;

    TemperatureHumidityReading read() override;
    const TemperatureHumidityReading& peek() const override;

    void clear() override;

    void setMockValues(
        float temperatureCelsius,
        float humidityPercent,
        bool valid = true
    );
};

#endif
