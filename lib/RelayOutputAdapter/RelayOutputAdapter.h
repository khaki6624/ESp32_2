#ifndef RELAY_OUTPUT_ADAPTER_H
#define RELAY_OUTPUT_ADAPTER_H

#include <OutputDriverPort.h>
#include <Relay.h>

class RelayOutputAdapter final : public OutputDriverPort
{
public:
    explicit RelayOutputAdapter(Relay& relay);

    DriverType getDriverType() const override;
    bool supportsAction(DeviceAction action) const override;
    DeviceValueType getValueType() const override;
    DriverPortHealth getHealth() const override;
    DriverExecutionResult execute(
        const DriverActionRequest& request,
        DriverExecutionResponse& response
    ) override;
    DriverExecutionResult readActualValue(DeviceValue& output) const override;

private:
    // Adapter مالک Driver نیست و Lifecycle آن توسط Composition Root مدیریت می‌شود.
    Relay& relay_;
};

#endif
