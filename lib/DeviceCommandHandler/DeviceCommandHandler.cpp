#include "DeviceCommandHandler.h"

DeviceCommandHandler::DeviceCommandHandler(
    const DeviceRegistry& deviceRegistry,
    const DeviceTemplateRegistry& templateRegistry,
    const OutputManager& outputManager
) :
    deviceRegistry_(deviceRegistry),
    templateRegistry_(templateRegistry),
    outputManager_(outputManager)
{
}

DeviceCommandHandlerResult DeviceCommandHandler::handle(
    const Command& command,
    CommandResult& output
) const
{
    if (!command.isValid())
        return DeviceCommandHandlerResult::INVALID_COMMAND;
    if (command.domain != CommandDomain::OUT)
        return DeviceCommandHandlerResult::INVALID_DOMAIN;
    if (command.isQuery())
        return DeviceCommandHandlerResult::INVALID_QUERY;
    if (!command.hasDomainIndex || command.domainIndex == 0U)
        return DeviceCommandHandlerResult::INVALID_DOMAIN_INDEX;
    if (command.operation == CommandOperation::NONE ||
        !isValidCommandOperation(command.operation))
        return DeviceCommandHandlerResult::INVALID_OPERATION;

    const Device* device = nullptr;
    const DeviceTemplate* deviceTemplate = nullptr;
    DeviceCommandHandlerResult result = resolveDeviceAndTemplate(
        command, device, deviceTemplate
    );
    if (result != DeviceCommandHandlerResult::SUCCESS)
        return result;

    DriverActionRequest request;
    result = mapAction(command, request);
    if (result != DeviceCommandHandlerResult::SUCCESS)
        return result;

    DriverExecutionResponse response;
    const DriverExecutionResult driverResult = outputManager_.execute(
        *device, *deviceTemplate, request, response
    );
    result = mapDriverExecutionResult(driverResult);
    if (result != DeviceCommandHandlerResult::SUCCESS)
        return result;

    CommandResult temporary;
    result = buildCommandResult(command, response, temporary);
    if (result != DeviceCommandHandlerResult::SUCCESS)
        return result;
    output = temporary;
    return DeviceCommandHandlerResult::SUCCESS;
}

DeviceCommandHandlerResult DeviceCommandHandler::resolveDeviceAndTemplate(
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
    if (resolvedDevice->templateId != resolvedTemplate->id)
        return DeviceCommandHandlerResult::DEVICE_TEMPLATE_MISMATCH;
    if (resolvedDevice->binding.driverType != DriverType::RELAY &&
        resolvedDevice->binding.driverType != DriverType::DIMMER &&
        resolvedDevice->binding.driverType != DriverType::ANALOG_OUTPUT)
        return DeviceCommandHandlerResult::DEVICE_TEMPLATE_MISMATCH;

    device = resolvedDevice;
    deviceTemplate = resolvedTemplate;
    return DeviceCommandHandlerResult::SUCCESS;
}

DeviceCommandHandlerResult DeviceCommandHandler::mapAction(
    const Command& command,
    DriverActionRequest& request
) const
{
    DriverActionRequest temporary;
    switch (command.operation)
    {
        case CommandOperation::ON:
            temporary.action = DeviceAction::ON;
            break;
        case CommandOperation::OFF:
            temporary.action = DeviceAction::OFF;
            break;
        case CommandOperation::TOGGLE:
            temporary.action = DeviceAction::TOGGLE;
            break;
        case CommandOperation::PULSE:
            temporary.action = DeviceAction::PULSE;
            break;
        case CommandOperation::STOP:
            temporary.action = DeviceAction::STOP;
            break;
        case CommandOperation::ENABLE:
            temporary.action = DeviceAction::ENABLE;
            break;
        case CommandOperation::DISABLE:
            temporary.action = DeviceAction::DISABLE;
            break;
        case CommandOperation::RESET:
            temporary.action = DeviceAction::RESET;
            break;
        default:
            return DeviceCommandHandlerResult::ACTION_MAPPING_FAILED;
    }

    if (command.argumentCount != 0U)
        return DeviceCommandHandlerResult::ARGUMENT_MAPPING_FAILED;
    if (temporary.action == DeviceAction::PULSE)
    {
        if (!command.hasDurationValue || command.durationMs == 0U)
            return DeviceCommandHandlerResult::DURATION_NOT_SUPPORTED;
        temporary.hasDuration = true;
        temporary.durationMs = command.durationMs;
    }
    else if (command.hasDurationValue)
    {
        return DeviceCommandHandlerResult::DURATION_NOT_SUPPORTED;
    }
    temporary.requestedTimestampMs = command.context.createdTimestampMs;
    if (!temporary.isValid())
        return DeviceCommandHandlerResult::ARGUMENT_MAPPING_FAILED;
    request = temporary;
    return DeviceCommandHandlerResult::SUCCESS;
}

DeviceCommandHandlerResult DeviceCommandHandler::buildCommandResult(
    const Command& command,
    const DriverExecutionResponse& response,
    CommandResult& output
) const
{
    if (!response.isValid() || response.result != DriverExecutionResult::SUCCESS)
        return DeviceCommandHandlerResult::COMMAND_RESULT_INVALID;

    CommandResult temporary;
    temporary.commandId = command.context.commandId;
    temporary.requestId = command.context.request.requestId;
    if (!temporary.isValid() ||
        !temporary.transitionTo(ExecutionStatus::VALIDATING, command.context.createdTimestampMs) ||
        !temporary.transitionTo(ExecutionStatus::ACCEPTED, command.context.createdTimestampMs) ||
        !temporary.transitionTo(ExecutionStatus::EXECUTING, command.context.createdTimestampMs))
        return DeviceCommandHandlerResult::COMMAND_RESULT_INVALID;
    if (response.hasActualValue)
        temporary.actualValue = response.actualValue;
    if (!temporary.transitionTo(
            ExecutionStatus::SUCCESS,
            response.completedTimestampMs,
            CommandErrorCode::NONE
        ) || !temporary.isValid() || !temporary.isSuccess())
        return DeviceCommandHandlerResult::COMMAND_RESULT_INVALID;

    output = temporary;
    return DeviceCommandHandlerResult::SUCCESS;
}
