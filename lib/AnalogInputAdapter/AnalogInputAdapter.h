#ifndef ANALOG_INPUT_ADAPTER_H
#define ANALOG_INPUT_ADAPTER_H

#include <InputDriverPort.h>
#include <AnalogInput.h>

class AnalogInputAdapter final : public InputDriverPort
{
public:
    explicit AnalogInputAdapter(AnalogInput& input);

    DriverType getDriverType() const override;
    DeviceValueType getValueType() const override;
    DriverPortHealth getHealth() const override;
    DriverExecutionResult read(DeviceValue& output) const override;

private:
    // Adapter مالک Driver نیست و begin/update آن را اجرا نمی‌کند.
    AnalogInput& input_;
};

#endif
