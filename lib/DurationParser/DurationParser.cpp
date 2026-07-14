#include "DurationParser.h"
DurationParser::DurationParser(){}
DurationParseResult DurationParser::parse(const char* text,uint32_t& output)const{if(!text)return DurationParseResult::EMPTY_VALUE;
 size_t length=0U;while(text[length]!='\0')++length;return parse(text,length,output);}
DurationParseResult DurationParser::parse(const char* text,size_t length,uint32_t& output)const
{if(!text||length==0U)return DurationParseResult::EMPTY_VALUE;size_t i=0U;uint32_t value=0U;
 if(text[0]<'0'||text[0]>'9')return DurationParseResult::INVALID_NUMBER;
 while(i<length&&text[i]>='0'&&text[i]<='9'){const uint32_t d=static_cast<uint32_t>(text[i]-'0');
  if(value>(UINT32_MAX-d)/10U)return DurationParseResult::OVERFLOW;value=value*10U+d;++i;}
 if(i==length)return DurationParseResult::INVALID_UNIT;uint32_t multiplier=0U;const char a=text[i]>='a'&&text[i]<='z'?text[i]-32:text[i];++i;
 if(a=='M'&&i<length&&((text[i]>='a'&&text[i]<='z'?text[i]-32:text[i])=='S')){multiplier=1U;++i;}
 else if(a=='S')multiplier=1000U;else if(a=='M')multiplier=60000U;else if(a=='H')multiplier=3600000U;
 else return (a=='.')?DurationParseResult::INVALID_NUMBER:DurationParseResult::INVALID_UNIT;
 if(i!=length)return DurationParseResult::TRAILING_DATA;if(value>UINT32_MAX/multiplier)return DurationParseResult::OVERFLOW;
 output=value*multiplier;return DurationParseResult::SUCCESS;}
