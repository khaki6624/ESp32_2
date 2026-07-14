#ifndef COMMAND_TEXT_READER_H
#define COMMAND_TEXT_READER_H
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
class CommandTextReader
{
public:
 explicit CommandTextReader(const char* text):text_(text),position_(0U),valid_(text!=nullptr){}
 bool isValid()const{return valid_;}bool isEnd()const{return !valid_||text_[position_]=='\0';}
 char peek()const{return isEnd()?'\0':text_[position_];}char read(){return isEnd()?'\0':text_[position_++];}
 size_t position()const{return position_;}void skipSpaces(){while(peek()==' '||peek()=='\t'||peek()=='\r'||peek()=='\n')++position_;}
 bool consume(char expected){if(peek()!=expected)return false;++position_;return true;}
 bool readIdentifier(char* output,size_t capacity){if(!valid_||!output||capacity==0U)return false;const size_t start=position_;
  const char first=peek();if(!isLetter(first)&&first!='_')return false;size_t length=0U;
  while(isLetter(peek())||isDigit(peek())||peek()=='_'){if(length+1U>=capacity){position_=start;return false;}output[length++]=read();}
  for(size_t i=length;i<capacity;++i)output[i]='\0';return true;}
 bool readUnsigned(uint32_t& output){if(!isDigit(peek()))return false;const size_t start=position_;uint32_t value=0U;
  while(isDigit(peek())){const uint32_t digit=static_cast<uint32_t>(peek()-'0');if(value>(UINT32_MAX-digit)/10U){position_=start;return false;}value=value*10U+digit;++position_;}output=value;return true;}
 bool readSigned(int32_t& output){const size_t start=position_;bool negative=false;if(peek()=='+'||peek()=='-')negative=read()=='-';
  uint32_t magnitude=0U;if(!readUnsigned(magnitude)){position_=start;return false;}const uint32_t limit=negative?2147483648UL:2147483647UL;
  if(magnitude>limit){position_=start;return false;}output=negative?(magnitude==2147483648UL?INT32_MIN:-static_cast<int32_t>(magnitude)):static_cast<int32_t>(magnitude);return true;}
 bool readFloat(float& output){const size_t start=position_;size_t p=position_;if(text_[p]=='+'||text_[p]=='-')++p;bool digit=false;
  while(isDigit(text_[p])){digit=true;++p;}if(text_[p]=='.'){++p;while(isDigit(text_[p])){digit=true;++p;}}
  if(!digit){position_=start;return false;}char buffer[32]{};const size_t length=p-start;if(length>=sizeof(buffer)){position_=start;return false;}
  for(size_t i=0U;i<length;++i)buffer[i]=text_[start+i];char* end=nullptr;const float value=strtof(buffer,&end);
  if(end!=buffer+length||!isfinite(value)){position_=start;return false;}position_=p;output=value;return true;}
private:
 static bool isLetter(char c){return(c>='A'&&c<='Z')||(c>='a'&&c<='z');}static bool isDigit(char c){return c>='0'&&c<='9';}
 const char* text_;size_t position_;bool valid_;
};
#endif
