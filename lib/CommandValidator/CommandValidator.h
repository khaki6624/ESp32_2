#ifndef COMMAND_VALIDATOR_H
#define COMMAND_VALIDATOR_H
#include <Command.h>
#include <CommandValidationCommon.h>
class CommandValidator{public:CommandValidator();CommandValidationResult validate(Command& command)const;
private:bool validateDomainIndex(const Command& command)const;CommandValidationResult validateQuery(const Command& command)const;
 CommandValidationResult validateAction(const Command& command)const;bool determineRisk(const Command& command,CommandRisk& output)const;};
#endif
