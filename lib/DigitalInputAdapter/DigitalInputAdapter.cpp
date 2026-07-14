#include "DigitalInputAdapter.h"

DigitalInputAdapter::DigitalInputAdapter(DigitalInput& input) : input_(input) {}

DriverType DigitalInputAdapter::getDriverType() const
{
    return DriverType::DIGITAL_INPUT;
}

DeviceValueType DigitalInputAdapter::getValueType() const
{
    return DeviceValueType::BOOLEAN;
}

DriverPortHealth DigitalInputAdapter::getHealth() const
{
    // DigitalInput فعلی قرارداد Health یا Initialized ندارد.
    return DriverPortHealth::READY;
}

DriverExecutionResult DigitalInputAdapter::read(DeviceValue& output) const
{
    // فقط آخرین وضعیت منطقی و Debounce‌شده Driver خوانده می‌شود.
    DeviceValue temporary = DeviceValue::makeBoolean(input_.isActive(), 0U);
    if (!isValidDriverDeviceValue(temporary) || !temporary.valid)
        return DriverExecutionResult::INPUT_VALUE_INVALID;
    output = temporary;
    return DriverExecutionResult::SUCCESS;
}
