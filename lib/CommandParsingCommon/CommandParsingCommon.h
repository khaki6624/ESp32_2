#ifndef COMMAND_PARSING_COMMON_H
#define COMMAND_PARSING_COMMON_H
#include <CommandCommon.h>
enum class CommandParseResult:uint8_t{SUCCESS=0,EMPTY_INPUT,INPUT_TOO_LONG,INVALID_CHARACTER,
 INVALID_DOMAIN,INVALID_DOMAIN_INDEX,INVALID_PATH,PATH_TOO_DEEP,MISSING_OPERATOR,MULTIPLE_OPERATORS,
 INVALID_QUERY,INVALID_OPERATION,INVALID_ARGUMENT,TOO_MANY_ARGUMENTS,INVALID_DURATION,
 DURATION_OVERFLOW,INVALID_CONFIRM_TOKEN,TRAILING_DATA,OUTPUT_COMMAND_INVALID};
inline bool isValidCommandParseResult(CommandParseResult v){switch(v){case CommandParseResult::SUCCESS:
 case CommandParseResult::EMPTY_INPUT:case CommandParseResult::INPUT_TOO_LONG:case CommandParseResult::INVALID_CHARACTER:
 case CommandParseResult::INVALID_DOMAIN:case CommandParseResult::INVALID_DOMAIN_INDEX:case CommandParseResult::INVALID_PATH:
 case CommandParseResult::PATH_TOO_DEEP:case CommandParseResult::MISSING_OPERATOR:case CommandParseResult::MULTIPLE_OPERATORS:
 case CommandParseResult::INVALID_QUERY:case CommandParseResult::INVALID_OPERATION:case CommandParseResult::INVALID_ARGUMENT:
 case CommandParseResult::TOO_MANY_ARGUMENTS:case CommandParseResult::INVALID_DURATION:case CommandParseResult::DURATION_OVERFLOW:
 case CommandParseResult::INVALID_CONFIRM_TOKEN:case CommandParseResult::TRAILING_DATA:
 case CommandParseResult::OUTPUT_COMMAND_INVALID:return true;default:return false;}}
enum class DurationParseResult:uint8_t{SUCCESS=0,EMPTY_VALUE,INVALID_NUMBER,INVALID_UNIT,ZERO_NOT_ALLOWED,OVERFLOW,TRAILING_DATA};
inline bool isValidDurationParseResult(DurationParseResult v){switch(v){case DurationParseResult::SUCCESS:
 case DurationParseResult::EMPTY_VALUE:case DurationParseResult::INVALID_NUMBER:case DurationParseResult::INVALID_UNIT:
 case DurationParseResult::ZERO_NOT_ALLOWED:case DurationParseResult::OVERFLOW:case DurationParseResult::TRAILING_DATA:return true;default:return false;}}
enum class CommandTokenType:uint8_t{NONE=0,IDENTIFIER,UNSIGNED_INTEGER,SIGNED_INTEGER,FLOAT_NUMBER,
 BOOLEAN_VALUE,DURATION_VALUE,QUERY_MARK,ASSIGN_MARK,DOT,COMMA};
inline bool isValidCommandTokenType(CommandTokenType v){switch(v){case CommandTokenType::NONE:case CommandTokenType::IDENTIFIER:
 case CommandTokenType::UNSIGNED_INTEGER:case CommandTokenType::SIGNED_INTEGER:case CommandTokenType::FLOAT_NUMBER:
 case CommandTokenType::BOOLEAN_VALUE:case CommandTokenType::DURATION_VALUE:case CommandTokenType::QUERY_MARK:
 case CommandTokenType::ASSIGN_MARK:case CommandTokenType::DOT:case CommandTokenType::COMMA:return true;default:return false;}}
#endif
