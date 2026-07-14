#include <CommandParser.h>
#include <CommandTextReader.h>
#include <CommandValidator.h>
#include <DurationParser.h>
#include <Arduino.h>
#include <unity.h>
static CommandParser parserValue;static CommandValidator validatorValue;static DurationParser durationValue;
static RequestContext requestValue(){RequestContext r;r.requestId=7U;r.source=CommandSource::SERIAL;return r;}
static CommandParseResult parse(const char* text,Command& output){return parserValue.parse(text,requestValue(),9U,123U,output);}
void testReader(){CommandTextReader invalid(nullptr);TEST_ASSERT_FALSE(invalid.isValid());char name[8]{};CommandTextReader id("OUT_1");
 TEST_ASSERT_TRUE(id.readIdentifier(name,sizeof(name)));TEST_ASSERT_EQUAL_STRING("OUT_1",name);CommandTextReader bad("1OUT");TEST_ASSERT_FALSE(bad.readIdentifier(name,sizeof(name)));
 uint32_t u=1U;CommandTextReader overflow("4294967296");TEST_ASSERT_FALSE(overflow.readUnsigned(u));TEST_ASSERT_EQUAL_UINT32(1U,u);
 int32_t s=0;CommandTextReader negative("-42");TEST_ASSERT_TRUE(negative.readSigned(s));TEST_ASSERT_EQUAL_INT32(-42,s);
 float f=0;CommandTextReader decimal("-1.5");TEST_ASSERT_TRUE(decimal.readFloat(f));TEST_ASSERT_FLOAT_WITHIN(0.001F,-1.5F,f);
 CommandTextReader nan("NaN");TEST_ASSERT_FALSE(nan.readFloat(f));CommandTextReader inf("Inf");TEST_ASSERT_FALSE(inf.readFloat(f));}
void testDuration(){uint32_t value=99U;TEST_ASSERT_TRUE(durationValue.parse("500MS",value)==DurationParseResult::SUCCESS);TEST_ASSERT_EQUAL_UINT32(500U,value);
 TEST_ASSERT_TRUE(durationValue.parse("30s",value)==DurationParseResult::SUCCESS);TEST_ASSERT_EQUAL_UINT32(30000U,value);
 TEST_ASSERT_TRUE(durationValue.parse("5M",value)==DurationParseResult::SUCCESS);TEST_ASSERT_EQUAL_UINT32(300000U,value);
 TEST_ASSERT_TRUE(durationValue.parse("2H",value)==DurationParseResult::SUCCESS);TEST_ASSERT_EQUAL_UINT32(7200000U,value);
 value=77U;TEST_ASSERT_TRUE(durationValue.parse("1.5S",value)!=DurationParseResult::SUCCESS);TEST_ASSERT_EQUAL_UINT32(77U,value);
 TEST_ASSERT_TRUE(durationValue.parse("500X",value)==DurationParseResult::INVALID_UNIT);TEST_ASSERT_TRUE(durationValue.parse("999999999H",value)==DurationParseResult::OVERFLOW);}
void testQueryAndActionParsing(){Command c;TEST_ASSERT_TRUE(parse("OUT?",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(c.queryType==CommandQueryType::ITEM_INFO);
 TEST_ASSERT_TRUE(parse("OUT??",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(c.queryType==CommandQueryType::LIST_ITEMS);
 TEST_ASSERT_TRUE(parse("NODE1.CAPS?",c)==CommandParseResult::SUCCESS);TEST_ASSERT_EQUAL_UINT16(1U,c.domainIndex);TEST_ASSERT_EQUAL_UINT8(1U,c.path.count);
 TEST_ASSERT_TRUE(parse("OUT???",c)==CommandParseResult::INVALID_QUERY);TEST_ASSERT_TRUE(parse("OUT?=ON",c)==CommandParseResult::INVALID_QUERY);
 TEST_ASSERT_TRUE(parse("out1=on",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(c.operation==CommandOperation::ON);TEST_ASSERT_EQUAL_UINT16(1U,c.domainIndex);
 TEST_ASSERT_TRUE(parse("OUT1=PULSE,500MS",c)==CommandParseResult::SUCCESS);TEST_ASSERT_EQUAL_UINT32(500U,c.durationMs);
 TEST_ASSERT_TRUE(parse("SYS=REBOOT,842913",c)==CommandParseResult::SUCCESS);TEST_ASSERT_EQUAL_UINT32(842913U,c.context.confirmToken);
 TEST_ASSERT_TRUE(parse("SYS=REBOOT,4294967296",c)==CommandParseResult::INVALID_CONFIRM_TOKEN);}
void testParserFailuresAreAtomic(){Command output;output.context.commandId=55U;TEST_ASSERT_TRUE(parse("",output)==CommandParseResult::EMPTY_INPUT);TEST_ASSERT_EQUAL_UINT32(55U,output.context.commandId);
 TEST_ASSERT_TRUE(parse("UNKNOWN1=ON",output)==CommandParseResult::INVALID_DOMAIN);TEST_ASSERT_TRUE(parse("OUT0=ON",output)==CommandParseResult::INVALID_DOMAIN_INDEX);
 TEST_ASSERT_TRUE(parse("OUT1==ON",output)==CommandParseResult::MULTIPLE_OPERATORS);TEST_ASSERT_TRUE(parse("OUT1=ON,",output)==CommandParseResult::TRAILING_DATA);
 TEST_ASSERT_TRUE(parse("OUT1 = ON",output)==CommandParseResult::INVALID_CHARACTER);}
void testValidatorPoliciesAndRisk(){Command c;TEST_ASSERT_TRUE(parse("OUT1?",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(validatorValue.validate(c)==CommandValidationResult::VALID);TEST_ASSERT_TRUE(c.context.risk==CommandRisk::SAFE);
 TEST_ASSERT_TRUE(parse("OUT1??",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(validatorValue.validate(c)==CommandValidationResult::DOMAIN_INDEX_FORBIDDEN);
 TEST_ASSERT_TRUE(parse("OUT=ON",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(validatorValue.validate(c)==CommandValidationResult::DOMAIN_INDEX_REQUIRED);
 TEST_ASSERT_TRUE(parse("OUT1=PULSE",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(validatorValue.validate(c)==CommandValidationResult::DURATION_REQUIRED);
 TEST_ASSERT_TRUE(parse("NODE1=PING",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(validatorValue.validate(c)==CommandValidationResult::VALID);TEST_ASSERT_TRUE(c.context.risk==CommandRisk::SAFE);
 TEST_ASSERT_TRUE(parse("NODE1=REBOOT",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(validatorValue.validate(c)==CommandValidationResult::VALID);TEST_ASSERT_TRUE(c.context.risk==CommandRisk::DANGEROUS);
 TEST_ASSERT_TRUE(parse("CALL=STOP",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(validatorValue.validate(c)==CommandValidationResult::VALID);
 TEST_ASSERT_TRUE(parse("CALL1=STOP",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(validatorValue.validate(c)==CommandValidationResult::DOMAIN_INDEX_FORBIDDEN);
 TEST_ASSERT_TRUE(parse("SYS=HEALTH",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(validatorValue.validate(c)==CommandValidationResult::VALID);TEST_ASSERT_TRUE(c.context.risk==CommandRisk::SAFE);
 TEST_ASSERT_TRUE(parse("SYS=REBOOT,42",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(validatorValue.validate(c)==CommandValidationResult::VALID);}
void testValidatorAtomicity(){Command c;TEST_ASSERT_TRUE(parse("OUT=ON",c)==CommandParseResult::SUCCESS);c.context.risk=CommandRisk::SENSITIVE;
 TEST_ASSERT_TRUE(validatorValue.validate(c)!=CommandValidationResult::VALID);TEST_ASSERT_TRUE(c.context.risk==CommandRisk::SENSITIVE);
 TEST_ASSERT_TRUE(parse("OUT1=ON",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(validatorValue.validate(c)==CommandValidationResult::VALID);TEST_ASSERT_TRUE(c.context.risk==CommandRisk::ACTION);}
void testUserClaimAndIndexedOperations(){Command c;TEST_ASSERT_TRUE(parse("USER=CLAIM",c)==CommandParseResult::SUCCESS);
 TEST_ASSERT_TRUE(validatorValue.validate(c)==CommandValidationResult::VALID);TEST_ASSERT_TRUE(c.context.risk==CommandRisk::SENSITIVE);
 TEST_ASSERT_TRUE(parse("USER1=CLAIM",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(validatorValue.validate(c)==CommandValidationResult::DOMAIN_INDEX_FORBIDDEN);
 const char* missing[]={"USER=ADD","USER=DEL","USER=ENABLE","USER=DISABLE"};
 const char* indexed[]={"USER1=ADD","USER1=DEL","USER1=ENABLE","USER1=DISABLE"};
 for(size_t i=0U;i<4U;++i){TEST_ASSERT_TRUE(parse(missing[i],c)==CommandParseResult::SUCCESS);
  TEST_ASSERT_TRUE(validatorValue.validate(c)==CommandValidationResult::DOMAIN_INDEX_REQUIRED);
  TEST_ASSERT_TRUE(parse(indexed[i],c)==CommandParseResult::SUCCESS);
  TEST_ASSERT_TRUE(validatorValue.validate(c)==CommandValidationResult::VALID);TEST_ASSERT_TRUE(c.context.risk==CommandRisk::SENSITIVE);}}
void testExplicitDomainMapping(){struct DomainCase{const char* text;CommandDomain domain;};static const DomainCase cases[]={
 {"OUT?",CommandDomain::OUT},{"IN?",CommandDomain::IN},{"ADC?",CommandDomain::ADC},{"IR?",CommandDomain::IR},
 {"RF?",CommandDomain::RF},{"NODE?",CommandDomain::NODE},{"CFG?",CommandDomain::CFG},{"SCN?",CommandDomain::SCN},
 {"RULE?",CommandDomain::RULE},{"SCH?",CommandDomain::SCH},{"SYS?",CommandDomain::SYS},{"NET?",CommandDomain::NET},
 {"SMS?",CommandDomain::SMS},{"CALL?",CommandDomain::CALL},{"LOG?",CommandDomain::LOG},{"STORE?",CommandDomain::STORE},
 {"TEST?",CommandDomain::TEST},{"EVENT?",CommandDomain::EVENT},{"TIME?",CommandDomain::TIME},{"USER?",CommandDomain::USER}};
 Command c;for(size_t i=0U;i<sizeof(cases)/sizeof(cases[0]);++i){TEST_ASSERT_TRUE(parse(cases[i].text,c)==CommandParseResult::SUCCESS);
  TEST_ASSERT_TRUE(c.domain==cases[i].domain);}TEST_ASSERT_TRUE(parse("user?",c)==CommandParseResult::SUCCESS);TEST_ASSERT_TRUE(c.domain==CommandDomain::USER);}
static_assert(sizeof(CommandTextReader)<=16U,"Reader بزرگ است");static_assert(sizeof(CommandParser)<=4U,"Parser بزرگ است");static_assert(sizeof(CommandValidator)<=4U,"Validator بزرگ است");
void setup(){UNITY_BEGIN();RUN_TEST(testReader);RUN_TEST(testDuration);RUN_TEST(testQueryAndActionParsing);RUN_TEST(testParserFailuresAreAtomic);RUN_TEST(testValidatorPoliciesAndRisk);RUN_TEST(testValidatorAtomicity);RUN_TEST(testUserClaimAndIndexedOperations);RUN_TEST(testExplicitDomainMapping);UNITY_END();}void loop(){}
