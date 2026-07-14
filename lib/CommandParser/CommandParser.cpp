#include "CommandParser.h"
#include <CommandTextReader.h>
#include <DurationParser.h>
#include <string.h>
namespace
{
char upper(char c){return c>='a'&&c<='z'?static_cast<char>(c-32):c;}
bool equalWord(const char* a,const char* b){size_t i=0U;while(a[i]&&b[i]){if(upper(a[i])!=b[i])return false;++i;}return a[i]=='\0'&&b[i]=='\0';}
bool mapDomain(const char* n,CommandDomain& out){const char* names[]={"OUT","IN","ADC","IR","RF","NODE","CFG","SCN","RULE","SCH","SYS","NET","SMS","CALL","LOG","STORE","TEST","EVENT","TIME","USER"};
 for(uint8_t i=0U;i<20U;++i)if(equalWord(n,names[i])){out=static_cast<CommandDomain>(i+1U);return true;}return false;}
bool mapOperation(const char* n,CommandOperation& o){struct Pair{const char* n;CommandOperation o;};static const Pair p[]={
 {"READ",CommandOperation::READ},{"ON",CommandOperation::ON},{"OFF",CommandOperation::OFF},{"TOGGLE",CommandOperation::TOGGLE},{"PULSE",CommandOperation::PULSE},
 {"LEARN",CommandOperation::LEARN},{"SEND",CommandOperation::SEND},{"DEL",CommandOperation::DELETE_ITEM},{"PING",CommandOperation::PING},
 {"REBOOT",CommandOperation::REBOOT},{"SYNC",CommandOperation::SYNC},{"DISCOVER",CommandOperation::DISCOVER},{"EXPORT",CommandOperation::EXPORT_DATA},
 {"IMPORT",CommandOperation::IMPORT_DATA},{"BACKUP",CommandOperation::BACKUP},{"RESTORE",CommandOperation::RESTORE},{"RESET",CommandOperation::RESET},
 {"CLEAR",CommandOperation::CLEAR},{"SAVE",CommandOperation::SAVE},{"LOAD",CommandOperation::LOAD},{"VERIFY",CommandOperation::VERIFY},
 {"RUN",CommandOperation::RUN},{"STOP",CommandOperation::STOP},{"ADD",CommandOperation::ADD},{"REMOVE_STEP",CommandOperation::REMOVE_STEP},
 {"RECONNECT",CommandOperation::RECONNECT},{"SCAN",CommandOperation::SCAN},{"START",CommandOperation::START},{"FLUSH",CommandOperation::FLUSH},
 {"TEST",CommandOperation::TEST_ACTION},{"SAFEBOOT",CommandOperation::SAFEBOOT},{"FACTORY",CommandOperation::FACTORY},{"OTA",CommandOperation::OTA},
 {"HEALTH",CommandOperation::HEALTH},{"SET",CommandOperation::SET},{"ENABLE",CommandOperation::ENABLE},{"DISABLE",CommandOperation::DISABLE},{"CLAIM",CommandOperation::CLAIM}};
 for(size_t i=0U;i<sizeof(p)/sizeof(p[0]);++i)if(equalWord(n,p[i].n)){o=p[i].o;return true;}return false;}
bool dangerous(CommandOperation o){return o==CommandOperation::REBOOT||o==CommandOperation::FACTORY||o==CommandOperation::OTA||
 o==CommandOperation::RESTORE||o==CommandOperation::RESET||o==CommandOperation::IMPORT_DATA||o==CommandOperation::CLEAR;}
bool allDigits(const char* s){if(!s[0])return false;for(size_t i=0U;s[i];++i)if(s[i]<'0'||s[i]>'9')return false;return true;}
bool parseU16Suffix(const char* source,char* name,size_t capacity,uint16_t& index,bool& hasIndex)
{size_t length=strlen(source),split=length;while(split>0U&&source[split-1U]>='0'&&source[split-1U]<='9')--split;
 if(split==0U||split>=capacity)return false;for(size_t i=0U;i<capacity;++i)name[i]='\0';for(size_t i=0U;i<split;++i)name[i]=source[i];
 hasIndex=split<length;index=0U;if(hasIndex){uint32_t value=0U;for(size_t i=split;i<length;++i){value=value*10U+static_cast<uint32_t>(source[i]-'0');if(value>65535U)return false;}if(value==0U)return false;index=static_cast<uint16_t>(value);}return true;}
}
CommandParser::CommandParser(){}
CommandParseResult CommandParser::parse(const char* text,const RequestContext& request,CommandId commandId,
 uint32_t createdTimestampMs,Command& output)const
{
 if(!text)return CommandParseResult::EMPTY_INPUT;size_t total=0U;while(total<COMMAND_ORIGINAL_TEXT_MAX_LENGTH&&text[total])++total;
 if(total>=COMMAND_ORIGINAL_TEXT_MAX_LENGTH)return CommandParseResult::INPUT_TOO_LONG;size_t begin=0U,end=total;
 while(begin<end&&(text[begin]==' '||text[begin]=='\t'||text[begin]=='\r'||text[begin]=='\n'))++begin;
 while(end>begin&&(text[end-1U]==' '||text[end-1U]=='\t'||text[end-1U]=='\r'||text[end-1U]=='\n'))--end;
 if(begin==end)return CommandParseResult::EMPTY_INPUT;for(size_t i=begin;i<end;++i)if(text[i]==' '||text[i]=='\t'||text[i]=='\r'||text[i]=='\n')return CommandParseResult::INVALID_CHARACTER;
 char input[COMMAND_ORIGINAL_TEXT_MAX_LENGTH]{};const size_t length=end-begin;for(size_t i=0U;i<length;++i)input[i]=text[begin+i];
 size_t equals=0U,questions=0U,operatorPos=length;for(size_t i=0U;i<length;++i){const char c=input[i];
  if(c=='='){++equals;operatorPos=i;}else if(c=='?')++questions;else if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_'||c=='.'||c==','||c=='-'||c=='+'||c=='%'))return CommandParseResult::INVALID_CHARACTER;}
 if(equals>1U)return CommandParseResult::MULTIPLE_OPERATORS;if(questions>0U&&equals>0U)return CommandParseResult::INVALID_QUERY;
 const bool query=questions>0U;if(!query&&equals==0U)return CommandParseResult::MISSING_OPERATOR;
 size_t targetEnd=query?length-questions:operatorPos;if(query&&(questions>2U||targetEnd==0U))return CommandParseResult::INVALID_QUERY;
 Command temporary;temporary.context.commandId=commandId;temporary.context.request=request;temporary.context.risk=CommandRisk::SAFE;
 temporary.context.priority=CommandPriority::NORMAL;temporary.context.createdTimestampMs=createdTimestampMs;
 char segment[COMMAND_PATH_SEGMENT_MAX_LENGTH]{};size_t pos=0U,segmentNumber=0U;
 while(pos<targetEnd){size_t start=pos;while(pos<targetEnd&&input[pos]!='.')++pos;const size_t count=pos-start;
  if(count==0U||count>=sizeof(segment))return CommandParseResult::INVALID_PATH;for(size_t i=0U;i<sizeof(segment);++i)segment[i]='\0';for(size_t i=0U;i<count;++i)segment[i]=input[start+i];
  char name[COMMAND_PATH_SEGMENT_MAX_LENGTH]{};uint16_t index=0U;bool hasIndex=false;if(!parseU16Suffix(segment,name,sizeof(name),index,hasIndex))return segmentNumber==0U?CommandParseResult::INVALID_DOMAIN_INDEX:CommandParseResult::INVALID_PATH;
  if(segmentNumber==0U){if(!mapDomain(name,temporary.domain))return CommandParseResult::INVALID_DOMAIN;temporary.domainIndex=index;temporary.hasDomainIndex=hasIndex;}
  else{if(temporary.path.isFull())return CommandParseResult::PATH_TOO_DEEP;if(!temporary.path.addSegment(name,index,hasIndex))return CommandParseResult::INVALID_PATH;}
  ++segmentNumber;if(pos<targetEnd)++pos;}
 if(query){temporary.queryType=questions==1U?CommandQueryType::ITEM_INFO:CommandQueryType::LIST_ITEMS;
  if(!temporary.setOriginalText(text)||!temporary.isValid())return CommandParseResult::OUTPUT_COMMAND_INVALID;output=temporary;return CommandParseResult::SUCCESS;}
 pos=operatorPos+1U;if(pos>=length)return CommandParseResult::INVALID_OPERATION;size_t opStart=pos;while(pos<length&&input[pos]!=',')++pos;
 if(pos==opStart||pos-opStart>=sizeof(segment))return CommandParseResult::INVALID_OPERATION;for(size_t i=0U;i<sizeof(segment);++i)segment[i]='\0';for(size_t i=opStart;i<pos;++i)segment[i-opStart]=input[i];
 if(!mapOperation(segment,temporary.operation))return CommandParseResult::INVALID_OPERATION;
 DurationParser durationParser;bool hasToken=false;
 while(pos<length){if(input[pos++]!=','||pos>=length)return CommandParseResult::TRAILING_DATA;const size_t start=pos;while(pos<length&&input[pos]!=',')++pos;
  const size_t count=pos-start;if(count==0U||count>=COMMAND_TEXT_ARGUMENT_MAX_LENGTH)return CommandParseResult::INVALID_ARGUMENT;
  char token[COMMAND_TEXT_ARGUMENT_MAX_LENGTH]{};for(size_t i=0U;i<count;++i)token[i]=input[start+i];uint32_t duration=0U;
  const char tokenLast=token[count-1U];const bool durationCandidate=(tokenLast>='A'&&tokenLast<='Z')||(tokenLast>='a'&&tokenLast<='z');
  const DurationParseResult dr=durationCandidate?durationParser.parse(token,duration):DurationParseResult::INVALID_UNIT;
  if(dr==DurationParseResult::SUCCESS){if(temporary.hasDurationValue)return CommandParseResult::INVALID_DURATION;temporary.setDuration(duration);continue;}
  if(durationCandidate&&dr==DurationParseResult::OVERFLOW)return CommandParseResult::DURATION_OVERFLOW;
  if(dangerous(temporary.operation)&&allDigits(token)){uint32_t value=0U;for(size_t i=0U;token[i];++i){const uint32_t d=token[i]-'0';if(value>(UINT32_MAX-d)/10U)return CommandParseResult::INVALID_CONFIRM_TOKEN;value=value*10U+d;}
   if(value==0U||hasToken)return CommandParseResult::INVALID_CONFIRM_TOKEN;temporary.context.confirmToken=value;hasToken=true;continue;}
  if(dangerous(temporary.operation)&&temporary.argumentCount==0U)return CommandParseResult::INVALID_CONFIRM_TOKEN;
  if(temporary.argumentCount>=COMMAND_MAX_ARGUMENTS)return CommandParseResult::TOO_MANY_ARGUMENTS;CommandArgument argument;
  if(equalWord(token,"TRUE")||equalWord(token,"ON"))argument=CommandArgument::makeBoolean(true);
  else if(equalWord(token,"FALSE")||equalWord(token,"OFF"))argument=CommandArgument::makeBoolean(false);
  else{const size_t tl=strlen(token);if(token[tl-1U]=='%'){token[tl-1U]='\0';int32_t v=0;CommandTextReader reader(token);if(!reader.readSigned(v)||!reader.isEnd())return CommandParseResult::INVALID_ARGUMENT;argument=CommandArgument::makePercentage(v);}
   else{CommandTextReader reader(token);int32_t integer=0;float floating=0.0F;if(reader.readSigned(integer)&&reader.isEnd())argument=CommandArgument::makeInteger(integer);
    else{CommandTextReader floatReader(token);if(floatReader.readFloat(floating)&&floatReader.isEnd())argument=CommandArgument::makeFloat(floating);else argument=CommandArgument::makeText(CommandArgumentType::TEXT,token);}}}
  if(!argument.isValid()||!temporary.addArgument(argument))return CommandParseResult::INVALID_ARGUMENT;}
 if(!temporary.setOriginalText(text)||!temporary.isValid())return CommandParseResult::OUTPUT_COMMAND_INVALID;output=temporary;return CommandParseResult::SUCCESS;
}
