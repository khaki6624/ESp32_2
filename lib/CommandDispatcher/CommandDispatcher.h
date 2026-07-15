#ifndef COMMAND_DISPATCHER_H
#define COMMAND_DISPATCHER_H

#include <stddef.h>

#include <CommandHandler.h>

constexpr size_t COMMAND_DISPATCHER_HANDLER_CAPACITY = 8;

struct CommandHandlerEntry
{
    const CommandHandler* handler;
    bool used;
};

class CommandDispatcher
{
public:
    CommandDispatcher();
    void clear();
    CommandDispatchResult registerHandler(const CommandHandler& handler);
    CommandDispatchResult unregisterHandler(const CommandHandler& handler);
    CommandDispatchResult dispatch(
        const Command& command,
        CommandResult& output
    ) const;
    size_t size() const;
    size_t capacity() const;

private:
    CommandHandlerEntry handlers_[COMMAND_DISPATCHER_HANDLER_CAPACITY];
    size_t count_;
};

#endif
