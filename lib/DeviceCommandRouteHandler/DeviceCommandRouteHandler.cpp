#include "DeviceCommandRouteHandler.h"

namespace
{
    CommandDispatchResult mapDeviceHandlerResult(DeviceCommandHandlerResult result)
    {
        switch (result)
        {
            case DeviceCommandHandlerResult::SUCCESS:
                return CommandDispatchResult::SUCCESS;
            case DeviceCommandHandlerResult::INVALID_COMMAND:
                return CommandDispatchResult::INVALID_COMMAND;
            case DeviceCommandHandlerResult::INVALID_DOMAIN:
                return CommandDispatchResult::INVALID_DOMAIN;
            case DeviceCommandHandlerResult::UNSUPPORTED_COMMAND:
                return CommandDispatchResult::UNSUPPORTED_DOMAIN;
            default:
                return CommandDispatchResult::HANDLER_REJECTED;
        }
    }
}

DeviceCommandRouteHandler::DeviceCommandRouteHandler(
    const DeviceCommandHandler& actionHandler,
    const DeviceQueryHandler& queryHandler
) : actionHandler_(actionHandler), queryHandler_(queryHandler) {}

bool DeviceCommandRouteHandler::supports(const Command& command) const
{
    return command.domain == CommandDomain::OUT ||
           command.domain == CommandDomain::IN ||
           command.domain == CommandDomain::ADC;
}

CommandDispatchResult DeviceCommandRouteHandler::handle(
    const Command& command,
    CommandResult& output
) const
{
    if (!command.isValid())
        return CommandDispatchResult::INVALID_COMMAND;
    if (!supports(command))
        return CommandDispatchResult::UNSUPPORTED_DOMAIN;

    CommandResult temporary;
    const DeviceCommandHandlerResult result = command.isQuery()
        ? queryHandler_.handle(command, temporary)
        : actionHandler_.handle(command, temporary);
    const CommandDispatchResult mapped = mapDeviceHandlerResult(result);
    if (mapped != CommandDispatchResult::SUCCESS)
        return mapped;
    if (!temporary.isValid() || !temporary.isTerminal())
        return CommandDispatchResult::RESULT_INVALID;
    output = temporary;
    return CommandDispatchResult::SUCCESS;
}
