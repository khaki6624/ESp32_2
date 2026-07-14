#ifndef DIGITAL_INPUT_ADAPTER_H
#define DIGITAL_INPUT_ADAPTER_H

#include <InputDriverPort.h>
#include <DigitalInput.h>

class DigitalInputAdapter final : public InputDriverPort
{
public:
    explicit DigitalInputAdapter(DigitalInput& input);

    DriverType getDriverType() const override;
    DeviceValueType getValueType() const override;
    DriverPortHealth getHealth() const override;
    DriverExecutionResult read(DeviceValue& output) const override;

private:
    // Adapter مالک Driver نیست و begin/update آن را اجرا نمی‌کند.
    DigitalInput& input_;
};

#endif
