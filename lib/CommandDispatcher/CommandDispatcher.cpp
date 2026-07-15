#include "CommandDispatcher.h"

CommandDispatcher::CommandDispatcher() : handlers_{}, count_(0U) {}

void CommandDispatcher::clear()
{
    for (size_t index = 0; index < COMMAND_DISPATCHER_HANDLER_CAPACITY; ++index)
        handlers_[index] = CommandHandlerEntry{};
    count_ = 0U;
}

CommandDispatchResult CommandDispatcher::registerHandler(
    const CommandHandler& handler
)
{
    size_t freeIndex = COMMAND_DISPATCHER_HANDLER_CAPACITY;
    for (size_t index = 0; index < COMMAND_DISPATCHER_HANDLER_CAPACITY; ++index)
    {
        if (handlers_[index].used)
        {
            if (handlers_[index].handler == &handler)
                return CommandDispatchResult::HANDLER_ALREADY_REGISTERED;
        }
        else if (freeIndex == COMMAND_DISPATCHER_HANDLER_CAPACITY)
        {
            freeIndex = index;
        }
    }
    if (freeIndex == COMMAND_DISPATCHER_HANDLER_CAPACITY)
        return CommandDispatchResult::DISPATCHER_FULL;
    handlers_[freeIndex].handler = &handler;
    handlers_[freeIndex].used = true;
    ++count_;
    return CommandDispatchResult::SUCCESS;
}

CommandDispatchResult CommandDispatcher::unregisterHandler(
    const CommandHandler& handler
)
{
    for (size_t index = 0; index < COMMAND_DISPATCHER_HANDLER_CAPACITY; ++index)
    {
        if (handlers_[index].used && handlers_[index].handler == &handler)
        {
            handlers_[index] = CommandHandlerEntry{};
            --count_;
            return CommandDispatchResult::SUCCESS;
        }
    }
    return CommandDispatchResult::HANDLER_NOT_REGISTERED;
}

CommandDispatchResult CommandDispatcher::dispatch(
    const Command& command,
    CommandResult& output
) const
{
    if (!command.isValid())
        return CommandDispatchResult::INVALID_COMMAND;
    if (command.domain == CommandDomain::NONE || !isValidCommandDomain(command.domain))
        return CommandDispatchResult::INVALID_DOMAIN;

    for (size_t index = 0; index < COMMAND_DISPATCHER_HANDLER_CAPACITY; ++index)
    {
        const CommandHandlerEntry& entry = handlers_[index];
        if (!entry.used || entry.handler == nullptr || !entry.handler->supports(command))
            continue;
        CommandResult temporary;
        const CommandDispatchResult result = entry.handler->handle(command, temporary);
        if (result != CommandDispatchResult::SUCCESS)
            return result;
        if (!temporary.isValid() || !temporary.isTerminal())
            return CommandDispatchResult::RESULT_INVALID;
        output = temporary;
        return CommandDispatchResult::SUCCESS;
    }
    return CommandDispatchResult::HANDLER_NOT_FOUND;
}

size_t CommandDispatcher::size() const { return count_; }
size_t CommandDispatcher::capacity() const
{
    return COMMAND_DISPATCHER_HANDLER_CAPACITY;
}
