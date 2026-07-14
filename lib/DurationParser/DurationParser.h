#ifndef DURATION_PARSER_H
#define DURATION_PARSER_H
#include <CommandParsingCommon.h>
class DurationParser{public:DurationParser();DurationParseResult parse(const char* text,uint32_t& outputMs)const;
 DurationParseResult parse(const char* text,size_t length,uint32_t& outputMs)const;};
#endif
