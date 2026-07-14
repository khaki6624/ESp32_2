#include "CommandValidator.h"
CommandValidator::CommandValidator(){}
bool CommandValidator::validateDomainIndex(const Command& c)const
{if(c.hasDomainIndex&&c.domainIndex==0U)return false;if(!c.hasDomainIndex&&c.domainIndex!=0U)return false;return true;}
CommandValidationResult CommandValidator::validate(Command& c)const
{if(!c.isValid())return CommandValidationResult::INVALID_COMMAND;if(!c.context.isValid())return CommandValidationResult::INVALID_CONTEXT;
 if(c.domain==CommandDomain::NONE||!isValidCommandDomain(c.domain))return CommandValidationResult::INVALID_DOMAIN;
 if(!validateDomainIndex(c))return CommandValidationResult::INVALID_DOMAIN_INDEX;if(!c.path.isValid())return CommandValidationResult::INVALID_PATH;
 CommandValidationResult result=c.isQuery()?validateQuery(c):validateAction(c);if(result!=CommandValidationResult::VALID)return result;
 CommandRisk risk;if(!determineRisk(c,risk))return CommandValidationResult::UNSUPPORTED_COMMAND;
 if(risk!=CommandRisk::DANGEROUS&&c.context.hasConfirmToken())return CommandValidationResult::CONFIRM_TOKEN_NOT_ALLOWED;
 c.context.risk=risk;return CommandValidationResult::VALID;}
CommandValidationResult CommandValidator::validateQuery(const Command& c)const
{if(c.queryType==CommandQueryType::NONE||c.operation!=CommandOperation::NONE)return CommandValidationResult::INVALID_QUERY;
 if(c.argumentCount!=0U)return CommandValidationResult::ARGUMENT_COUNT_MISMATCH;if(c.hasDurationValue)return CommandValidationResult::DURATION_NOT_ALLOWED;
 if(c.context.hasConfirmToken())return CommandValidationResult::CONFIRM_TOKEN_NOT_ALLOWED;
 if(c.queryType==CommandQueryType::LIST_ITEMS&&c.hasDomainIndex)return CommandValidationResult::DOMAIN_INDEX_FORBIDDEN;return CommandValidationResult::VALID;}
CommandValidationResult CommandValidator::validateAction(const Command& c)const
{if(c.operation==CommandOperation::NONE||!isValidCommandOperation(c.operation))return CommandValidationResult::INVALID_OPERATION;
 bool required=false,forbidden=false;switch(c.domain){case CommandDomain::OUT:case CommandDomain::IR:case CommandDomain::RF:
 case CommandDomain::NODE:case CommandDomain::SCN:case CommandDomain::RULE:case CommandDomain::SCH:required=true;break;
 case CommandDomain::SYS:case CommandDomain::STORE:case CommandDomain::TIME:case CommandDomain::LOG:case CommandDomain::EVENT:forbidden=true;break;
 case CommandDomain::SMS:required=c.operation==CommandOperation::SEND;forbidden=!required;break;
 case CommandDomain::CALL:required=c.operation==CommandOperation::START;forbidden=c.operation==CommandOperation::STOP;break;
 case CommandDomain::USER:required=c.operation==CommandOperation::ADD||
  c.operation==CommandOperation::DELETE_ITEM||c.operation==CommandOperation::ENABLE||
  c.operation==CommandOperation::DISABLE;forbidden=c.operation==CommandOperation::CLAIM;break;default:break;}
 if(required&&!c.hasDomainIndex)return CommandValidationResult::DOMAIN_INDEX_REQUIRED;if(forbidden&&c.hasDomainIndex)return CommandValidationResult::DOMAIN_INDEX_FORBIDDEN;
 bool compatible=false;switch(c.domain){case CommandDomain::OUT:compatible=c.operation==CommandOperation::ON||c.operation==CommandOperation::OFF||c.operation==CommandOperation::TOGGLE||c.operation==CommandOperation::PULSE;break;
 case CommandDomain::IN:case CommandDomain::ADC:compatible=false;break;case CommandDomain::IR:case CommandDomain::RF:compatible=c.operation==CommandOperation::LEARN||c.operation==CommandOperation::SEND||c.operation==CommandOperation::DELETE_ITEM;break;
 case CommandDomain::NODE:compatible=c.operation==CommandOperation::PING||c.operation==CommandOperation::REBOOT||c.operation==CommandOperation::SYNC||c.operation==CommandOperation::DISCOVER;break;
 case CommandDomain::SCN:case CommandDomain::SCH:compatible=c.operation==CommandOperation::RUN||c.operation==CommandOperation::STOP||c.operation==CommandOperation::DELETE_ITEM;break;
 case CommandDomain::RULE:compatible=c.operation==CommandOperation::DELETE_ITEM;break;case CommandDomain::SYS:compatible=c.operation==CommandOperation::REBOOT||c.operation==CommandOperation::SAFEBOOT||c.operation==CommandOperation::FACTORY||c.operation==CommandOperation::OTA||c.operation==CommandOperation::SAVE||c.operation==CommandOperation::SYNC||c.operation==CommandOperation::HEALTH;break;
 case CommandDomain::SMS:compatible=c.operation==CommandOperation::SEND||c.operation==CommandOperation::TEST_ACTION||c.operation==CommandOperation::FLUSH;break;
 case CommandDomain::CALL:compatible=c.operation==CommandOperation::START||c.operation==CommandOperation::STOP||c.operation==CommandOperation::TEST_ACTION;break;
 case CommandDomain::LOG:compatible=c.operation==CommandOperation::CLEAR||c.operation==CommandOperation::EXPORT_DATA;break;
 case CommandDomain::STORE:compatible=c.operation==CommandOperation::SAVE||c.operation==CommandOperation::LOAD||c.operation==CommandOperation::BACKUP||c.operation==CommandOperation::RESTORE||c.operation==CommandOperation::CLEAR||c.operation==CommandOperation::VERIFY;break;
 case CommandDomain::TIME:compatible=c.operation==CommandOperation::SYNC||c.operation==CommandOperation::SET;break;
 case CommandDomain::USER:compatible=c.operation==CommandOperation::CLAIM||c.operation==CommandOperation::ADD||c.operation==CommandOperation::DELETE_ITEM||c.operation==CommandOperation::ENABLE||c.operation==CommandOperation::DISABLE;break;
 case CommandDomain::NET:compatible=c.operation==CommandOperation::RECONNECT||c.operation==CommandOperation::RESET||c.operation==CommandOperation::SCAN;break;
 case CommandDomain::TEST:compatible=c.operation==CommandOperation::TEST_ACTION;break;
 case CommandDomain::CFG:compatible=c.operation==CommandOperation::EXPORT_DATA||c.operation==CommandOperation::IMPORT_DATA||c.operation==CommandOperation::BACKUP||c.operation==CommandOperation::RESTORE||c.operation==CommandOperation::RESET;break;
 default:compatible=false;break;}if(!compatible)return CommandValidationResult::DOMAIN_OPERATION_MISMATCH;
 if(c.argumentCount!=0U)return CommandValidationResult::ARGUMENT_COUNT_MISMATCH;
 if(c.hasDurationValue){if(c.durationMs==0U)return CommandValidationResult::INVALID_DURATION;if(c.domain!=CommandDomain::OUT||!(c.operation==CommandOperation::ON||c.operation==CommandOperation::OFF||c.operation==CommandOperation::TOGGLE||c.operation==CommandOperation::PULSE))return CommandValidationResult::DURATION_NOT_ALLOWED;}
 if(c.domain==CommandDomain::OUT&&c.operation==CommandOperation::PULSE&&!c.hasDurationValue)return CommandValidationResult::DURATION_REQUIRED;
 return CommandValidationResult::VALID;}
bool CommandValidator::determineRisk(const Command& c,CommandRisk& r)const
{if(c.isQuery()||(c.domain==CommandDomain::NODE&&(c.operation==CommandOperation::PING||c.operation==CommandOperation::DISCOVER))||(c.domain==CommandDomain::SYS&&c.operation==CommandOperation::HEALTH)){r=CommandRisk::SAFE;return true;}
 if((c.domain==CommandDomain::NODE&&c.operation==CommandOperation::REBOOT)||(c.domain==CommandDomain::SYS&&(c.operation==CommandOperation::REBOOT||c.operation==CommandOperation::FACTORY||c.operation==CommandOperation::OTA))||(c.domain==CommandDomain::STORE&&(c.operation==CommandOperation::CLEAR||c.operation==CommandOperation::RESTORE))||(c.domain==CommandDomain::CFG&&(c.operation==CommandOperation::RESET||c.operation==CommandOperation::RESTORE||c.operation==CommandOperation::IMPORT_DATA))){r=CommandRisk::DANGEROUS;return true;}
 if(((c.domain==CommandDomain::IR||c.domain==CommandDomain::RF)&&(c.operation==CommandOperation::LEARN||c.operation==CommandOperation::DELETE_ITEM))||(c.domain==CommandDomain::USER&&(c.operation==CommandOperation::CLAIM||c.operation==CommandOperation::ADD||c.operation==CommandOperation::DELETE_ITEM||c.operation==CommandOperation::ENABLE||c.operation==CommandOperation::DISABLE))||(c.domain==CommandDomain::RULE&&c.operation==CommandOperation::DELETE_ITEM)||(c.domain==CommandDomain::SCH&&c.operation==CommandOperation::DELETE_ITEM)){r=CommandRisk::SENSITIVE;return true;}
 r=CommandRisk::ACTION;return true;}
