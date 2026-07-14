#include "DeviceQueryHandler.h"

namespace
{
    bool domainMatchesDriver(CommandDomain domain, DriverType driverType)
    {
        switch (domain)
        {
            case CommandDomain::OUT:
                return driverType == DriverType::RELAY ||
                       driverType == DriverType::DIMMER ||
                       driverType == DriverType::ANALOG_OUTPUT;
            case CommandDomain::IN:
                return driverType == DriverType::DIGITAL_INPUT;
            case CommandDomain::ADC:
                return driverType == DriverType::ANALOG_INPUT;
            default:
                return false;
        }
    }

    bool buildQueryCommandResult(
        const Command& command,
        const DeviceValue& value,
        CommandResult& output
    )
    {
        if (!isValidDriverDeviceValue(value) || !value.valid)
            return false;
        CommandResult temporary;
        temporary.commandId = command.context.commandId;
        temporary.requestId = command.context.request.requestId;
        if (!temporary.isValid() ||
            !temporary.transitionTo(ExecutionStatus::VALIDATING, command.context.createdTimestampMs) ||
            !temporary.transitionTo(ExecutionStatus::ACCEPTED, command.context.createdTimestampMs) ||
            !temporary.transitionTo(ExecutionStatus::EXECUTING, command.context.createdTimestampMs))
            return false;
        temporary.actualValue = value;
        if (!temporary.transitionTo(
                ExecutionStatus::SUCCESS,
                value.timestampMs,
                CommandErrorCode::NONE
            ) || !temporary.isValid() || !temporary.isSuccess())
            return false;
        output = temporary;
        return true;
    }
}

DeviceQueryHandler::DeviceQueryHandler(
    const DeviceRegistry& deviceRegistry,
    const DeviceTemplateRegistry& templateRegistry,
    const OutputManager& outputManager,
    const InputManager& inputManager
) :
    deviceRegistry_(deviceRegistry),
    templateRegistry_(templateRegistry),
    outputManager_(outputManager),
    inputManager_(inputManager)
{
}

DeviceCommandHandlerResult DeviceQueryHandler::handle(
    const Command& command,
    CommandResult& output
) const
{
    if (!command.isValid())
        return DeviceCommandHandlerResult::INVALID_COMMAND;
    if (commandDomainToDeviceDomainKind(command.domain) == DeviceDomainKind::NONE)
        return DeviceCommandHandlerResult::INVALID_DOMAIN;
    if (!command.isQuery())
        return DeviceCommandHandlerResult::INVALID_QUERY;

    // Queryهای لیستی به Collection Result یا Serializer نیاز دارند و اینجا پشتیبانی نمی‌شوند.
    if (command.queryType == CommandQueryType::LIST_ITEMS)
        return DeviceCommandHandlerResult::UNSUPPORTED_COMMAND;
    if (command.queryType != CommandQueryType::ITEM_INFO)
        return DeviceCommandHandlerResult::INVALID_QUERY;
    if (!command.path.isEmpty())
        return DeviceCommandHandlerResult::UNSUPPORTED_COMMAND;
    if (!command.hasDomainIndex || command.domainIndex == 0U)
        return DeviceCommandHandlerResult::INVALID_DOMAIN_INDEX;

    const Device* device = nullptr;
    const DeviceTemplate* deviceTemplate = nullptr;
    DeviceCommandHandlerResult result = resolveDeviceAndTemplate(
        command, device, deviceTemplate
    );
    if (result != DeviceCommandHandlerResult::SUCCESS)
        return result;

    DeviceValue value;
    DriverExecutionResult driverResult = DriverExecutionResult::DRIVER_ERROR;
    switch (command.domain)
    {
        case CommandDomain::OUT:
            driverResult = outputManager_.readActualValue(
                *device, *deviceTemplate, value
            );
            break;
        case CommandDomain::IN:
        case CommandDomain::ADC:
            driverResult = inputManager_.read(*device, *deviceTemplate, value);
            break;
        default:
            return DeviceCommandHandlerResult::INVALID_DOMAIN;
    }
    if (driverResult != DriverExecutionResult::SUCCESS)
        return DeviceCommandHandlerResult::DRIVER_READ_FAILED;
    if (!buildQueryCommandResult(command, value, output))
        return DeviceCommandHandlerResult::COMMAND_RESULT_INVALID;
    return DeviceCommandHandlerResult::SUCCESS;
}

DeviceCommandHandlerResult DeviceQueryHandler::resolveDeviceAndTemplate(
    const Command& command,
    const Device*& device,
    const DeviceTemplate*& deviceTemplate
) const
{
    const DeviceId deviceId = static_cast<DeviceId>(command.domainIndex);
    const Device* resolvedDevice = deviceRegistry_.findById(deviceId);
    if (resolvedDevice == nullptr)
        return DeviceCommandHandlerResult::DEVICE_NOT_FOUND;
    const DeviceTemplate* resolvedTemplate = templateRegistry_.findById(
        resolvedDevice->templateId
    );
    if (resolvedTemplate == nullptr)
        return DeviceCommandHandlerResult::TEMPLATE_NOT_FOUND;
    if (resolvedDevice->templateId != resolvedTemplate->id ||
        !domainMatchesDriver(command.domain, resolvedDevice->binding.driverType))
        return DeviceCommandHandlerResult::DEVICE_TEMPLATE_MISMATCH;

    device = resolvedDevice;
    deviceTemplate = resolvedTemplate;
    return DeviceCommandHandlerResult::SUCCESS;
}
