#ifndef DEVICE_QUERY_HANDLER_H
#define DEVICE_QUERY_HANDLER_H

#include <Command.h>
#include <CommandResult.h>
#include <DeviceCommandCommon.h>
#include <DeviceRegistry.h>
#include <InputManager.h>
#include <OutputManager.h>

class DeviceQueryHandler
{
public:
    DeviceQueryHandler(
        const DeviceRegistry& deviceRegistry,
        const DeviceTemplateRegistry& templateRegistry,
        const OutputManager& outputManager,
        const InputManager& inputManager
    );

    DeviceCommandHandlerResult handle(
        const Command& command,
        CommandResult& output
    ) const;

private:
    const DeviceRegistry& deviceRegistry_;
    const DeviceTemplateRegistry& templateRegistry_;
    const OutputManager& outputManager_;
    const InputManager& inputManager_;

    DeviceCommandHandlerResult resolveDeviceAndTemplate(
        const Command& command,
        const Device*& device,
        const DeviceTemplate*& deviceTemplate
    ) const;
};

#endif
