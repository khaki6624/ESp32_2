#ifndef COMMAND_ID_PROVIDER_H
#define COMMAND_ID_PROVIDER_H

#include <CommandCommon.h>
#include <RuleExecutionCommon.h>

class CommandIdProvider
{
public:
    virtual ~CommandIdProvider() = default;
    virtual CommandIdReservationResult reserveRange(size_t count, CommandId& firstCommandId) = 0;
};

#endif
