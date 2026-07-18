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
    for (size_t index = 0; index < count_; ++index)
    {
        if (handlers_[index].handler == &handler)
            return CommandDispatchResult::HANDLER_ALREADY_REGISTERED;
    }
    if (count_ == COMMAND_DISPATCHER_HANDLER_CAPACITY)
        return CommandDispatchResult::DISPATCHER_FULL;
    handlers_[count_].handler = &handler;
    handlers_[count_].used = true;
    ++count_;
    return CommandDispatchResult::SUCCESS;
}

CommandDispatchResult CommandDispatcher::unregisterHandler(
    const CommandHandler& handler
)
{
    for (size_t index = 0; index < count_; ++index)
    {
        if (handlers_[index].handler == &handler)
        {
            for (size_t shiftIndex = index + 1U; shiftIndex < count_; ++shiftIndex)
                handlers_[shiftIndex - 1U] = handlers_[shiftIndex];
            --count_;
            handlers_[count_] = CommandHandlerEntry{};
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

    for (size_t index = 0; index < count_; ++index)
    {
        const CommandHandlerEntry& entry = handlers_[index];
        if (!entry.used || entry.handler == nullptr || !entry.handler->supports(command))
            continue;
        CommandResult temporary;
        const CommandDispatchResult result = entry.handler->handle(command, temporary);
        if (result != CommandDispatchResult::SUCCESS)
            return result;
        if (!temporary.isValid() || !temporary.isTerminal() || !temporary.isSuccess() ||
            temporary.commandId != command.context.commandId ||
            temporary.requestId != command.context.request.requestId)
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
