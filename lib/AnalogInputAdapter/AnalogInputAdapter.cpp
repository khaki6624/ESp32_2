#include "AnalogInputAdapter.h"

AnalogInputAdapter::AnalogInputAdapter(AnalogInput& input) : input_(input) {}

DriverType AnalogInputAdapter::getDriverType() const
{
    return DriverType::ANALOG_INPUT;
}

DeviceValueType AnalogInputAdapter::getValueType() const
{
    // Raw ADC قرارداد عمومی و بدون معنای Calibration برای AnalogInput است.
    return DeviceValueType::INTEGER;
}

DriverPortHealth AnalogInputAdapter::getHealth() const
{
    // AnalogInput فعلی قرارداد Health یا Initialized ندارد.
    return DriverPortHealth::READY;
}

DriverExecutionResult AnalogInputAdapter::read(DeviceValue& output) const
{
    DeviceValue temporary = DeviceValue::makeInteger(
        static_cast<int32_t>(input_.getRaw()),
        0U
    );
    if (!isValidDriverDeviceValue(temporary) || !temporary.valid)
        return DriverExecutionResult::INPUT_VALUE_INVALID;
    output = temporary;
    return DriverExecutionResult::SUCCESS;
}
