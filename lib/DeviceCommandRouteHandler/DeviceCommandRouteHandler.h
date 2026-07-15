#ifndef DEVICE_COMMAND_ROUTE_HANDLER_H
#define DEVICE_COMMAND_ROUTE_HANDLER_H

#include <CommandHandler.h>
#include <DeviceCommandHandler.h>
#include <DeviceQueryHandler.h>

class DeviceCommandRouteHandler final : public CommandHandler
{
public:
    DeviceCommandRouteHandler(
        const DeviceCommandHandler& actionHandler,
        const DeviceQueryHandler& queryHandler
    );
    bool supports(const Command& command) const override;
    CommandDispatchResult handle(
        const Command& command,
        CommandResult& output
    ) const override;

private:
    const DeviceCommandHandler& actionHandler_;
    const DeviceQueryHandler& queryHandler_;
};

#endif
