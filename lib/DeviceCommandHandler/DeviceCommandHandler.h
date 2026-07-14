#ifndef DEVICE_COMMAND_HANDLER_H
#define DEVICE_COMMAND_HANDLER_H

#include <Command.h>
#include <CommandResult.h>
#include <DeviceCommandCommon.h>
#include <DeviceRegistry.h>
#include <OutputManager.h>

class DeviceCommandHandler
{
public:
    DeviceCommandHandler(
        const DeviceRegistry& deviceRegistry,
        const DeviceTemplateRegistry& templateRegistry,
        const OutputManager& outputManager
    );

    DeviceCommandHandlerResult handle(
        const Command& command,
        CommandResult& output
    ) const;

private:
    const DeviceRegistry& deviceRegistry_;
    const DeviceTemplateRegistry& templateRegistry_;
    const OutputManager& outputManager_;

    DeviceCommandHandlerResult resolveDeviceAndTemplate(
        const Command& command,
        const Device*& device,
        const DeviceTemplate*& deviceTemplate
    ) const;
    DeviceCommandHandlerResult mapAction(
        const Command& command,
        DriverActionRequest& request
    ) const;
    DeviceCommandHandlerResult buildCommandResult(
        const Command& command,
        const DriverExecutionResponse& response,
        CommandResult& output
    ) const;
};

#endif
