#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H
#include <Command.h>
#include <CommandParsingCommon.h>
class CommandParser{public:CommandParser();CommandParseResult parse(const char* text,const RequestContext& request,
 CommandId commandId,uint32_t createdTimestampMs,Command& output)const;};
#endif
