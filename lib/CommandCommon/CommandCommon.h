#ifndef COMMAND_COMMON_H
#define COMMAND_COMMON_H

#include <stddef.h>
#include <stdint.h>

using CommandId = uint32_t;
using RequestId = uint32_t;
using UserId = uint16_t;
using SessionId = uint32_t;
using ConfirmToken = uint32_t;

constexpr CommandId INVALID_COMMAND_ID = 0;
constexpr RequestId INVALID_REQUEST_ID = 0;
constexpr UserId INVALID_USER_ID = 0;
constexpr SessionId INVALID_SESSION_ID = 0;
constexpr ConfirmToken INVALID_CONFIRM_TOKEN = 0;

constexpr size_t COMMAND_SOURCE_NAME_MAX_LENGTH = 24;
constexpr size_t COMMAND_ORIGINAL_TEXT_MAX_LENGTH = 128;
constexpr size_t COMMAND_PATH_SEGMENT_MAX_LENGTH = 24;
constexpr size_t COMMAND_TEXT_ARGUMENT_MAX_LENGTH = 96;
constexpr size_t COMMAND_RESULT_MESSAGE_MAX_LENGTH = 64;
constexpr size_t COMMAND_PATH_MAX_SEGMENTS = 5;
constexpr size_t COMMAND_MAX_ARGUMENTS = 4;

enum class CommandDomain : uint8_t
{
    NONE=0, OUT=1, IN=2, ADC=3, IR=4, RF=5, NODE=6, CFG=7, SCN=8,
    RULE=9, SCH=10, SYS=11, NET=12, SMS=13, CALL=14, LOG=15,
    STORE=16, TEST=17, EVENT=18, TIME=19, USER=20
};
inline bool isValidCommandDomain(CommandDomain value)
{ return static_cast<uint8_t>(value)<=20U; }

enum class CommandQueryType : uint8_t { NONE=0, ITEM_INFO=1, LIST_ITEMS=2 };
inline bool isValidCommandQueryType(CommandQueryType value)
{ return static_cast<uint8_t>(value)<=2U; }

enum class CommandOperation : uint16_t
{
    NONE=0, READ=100, ON=200, OFF=201, TOGGLE=202, PULSE=203,
    LEARN=300, SEND=301, DELETE_ITEM=302,
    PING=400, REBOOT=401, SYNC=402, DISCOVER=403,
    EXPORT_DATA=500, IMPORT_DATA=501, BACKUP=502, RESTORE=503, RESET=504,
    CLEAR=505, SAVE=506, LOAD=507, VERIFY=508,
    RUN=600, STOP=601, ADD=602, REMOVE_STEP=603,
    RECONNECT=700, SCAN=701, START=800, FLUSH=801, TEST_ACTION=802,
    SAFEBOOT=900, FACTORY=901, OTA=902, HEALTH=903,
    SET=1000, ENABLE=1001, DISABLE=1002, CLAIM=1003
};
inline bool isValidCommandOperation(CommandOperation value)
{
    switch(value)
    {
        case CommandOperation::NONE: case CommandOperation::READ:
        case CommandOperation::ON: case CommandOperation::OFF: case CommandOperation::TOGGLE:
        case CommandOperation::PULSE: case CommandOperation::LEARN: case CommandOperation::SEND:
        case CommandOperation::DELETE_ITEM: case CommandOperation::PING: case CommandOperation::REBOOT:
        case CommandOperation::SYNC: case CommandOperation::DISCOVER: case CommandOperation::EXPORT_DATA:
        case CommandOperation::IMPORT_DATA: case CommandOperation::BACKUP: case CommandOperation::RESTORE:
        case CommandOperation::RESET: case CommandOperation::CLEAR: case CommandOperation::SAVE:
        case CommandOperation::LOAD: case CommandOperation::VERIFY: case CommandOperation::RUN:
        case CommandOperation::STOP: case CommandOperation::ADD: case CommandOperation::REMOVE_STEP:
        case CommandOperation::RECONNECT: case CommandOperation::SCAN: case CommandOperation::START:
        case CommandOperation::FLUSH: case CommandOperation::TEST_ACTION: case CommandOperation::SAFEBOOT:
        case CommandOperation::FACTORY: case CommandOperation::OTA: case CommandOperation::HEALTH:
        case CommandOperation::SET: case CommandOperation::ENABLE: case CommandOperation::DISABLE:
        case CommandOperation::CLAIM: return true;
        default: return false;
    }
}

enum class CommandArgumentType : uint8_t
{ NONE=0, BOOLEAN, INTEGER, FLOAT, PERCENTAGE, DURATION_MS, IDENTIFIER, TOKEN, DATE, TIME, TEXT, COMMAND_TEXT };
inline bool isValidCommandArgumentType(CommandArgumentType value)
{ return static_cast<uint8_t>(value)<=11U; }

enum class CommandRisk : uint8_t { SAFE=0, ACTION, SENSITIVE, DANGEROUS };
inline bool isValidCommandRisk(CommandRisk value) { return static_cast<uint8_t>(value)<=3U; }
enum class CommandPriority : uint8_t { LOW=0, NORMAL, HIGH, EMERGENCY };
inline bool isValidCommandPriority(CommandPriority value) { return static_cast<uint8_t>(value)<=3U; }
enum class CommandSource : uint8_t
{ NONE=0, APP, MQTT, LOCAL_WIFI, SMS, SERIAL, RULE_ENGINE, SCENE, SCHEDULER, PHYSICAL_INPUT, SYSTEM };
inline bool isValidCommandSource(CommandSource value) { return static_cast<uint8_t>(value)<=10U; }
enum class SystemMode : uint8_t { NORMAL=0, LOCKED, MAINTENANCE, UPDATING, EMERGENCY_LOCK };
inline bool isValidSystemMode(SystemMode value) { return static_cast<uint8_t>(value)<=4U; }

enum class ExecutionStatus : uint8_t
{ RECEIVED=0, VALIDATING, ACCEPTED, EXECUTING, SUCCESS, FAILED, REJECTED, TIMEOUT, CANCELLED };
inline bool isValidExecutionStatus(ExecutionStatus value) { return static_cast<uint8_t>(value)<=8U; }
inline bool isTerminalExecutionStatus(ExecutionStatus value)
{
    return value==ExecutionStatus::SUCCESS || value==ExecutionStatus::FAILED ||
           value==ExecutionStatus::REJECTED || value==ExecutionStatus::TIMEOUT ||
           value==ExecutionStatus::CANCELLED;
}
inline bool isValidExecutionStatusTransition(ExecutionStatus from,ExecutionStatus to)
{
    if(!isValidExecutionStatus(from)||!isValidExecutionStatus(to)) return false;
    if(from==ExecutionStatus::RECEIVED) return to==ExecutionStatus::VALIDATING||to==ExecutionStatus::REJECTED;
    if(from==ExecutionStatus::VALIDATING) return to==ExecutionStatus::ACCEPTED||to==ExecutionStatus::REJECTED;
    if(from==ExecutionStatus::ACCEPTED) return to==ExecutionStatus::EXECUTING||to==ExecutionStatus::CANCELLED;
    if(from==ExecutionStatus::EXECUTING) return to==ExecutionStatus::SUCCESS||to==ExecutionStatus::FAILED||
        to==ExecutionStatus::TIMEOUT||to==ExecutionStatus::CANCELLED;
    return false;
}

enum class CommandErrorCode : uint16_t
{
    NONE=0, INVALID_COMMAND, INVALID_COMMAND_ID, INVALID_REQUEST_ID, INVALID_SOURCE,
    INVALID_DOMAIN, INVALID_PATH, INVALID_QUERY, INVALID_OPERATION, INVALID_ARGUMENT,
    TOO_MANY_ARGUMENTS, ARGUMENT_TYPE_MISMATCH, INVALID_DURATION,
    SYSTEM_LOCKED, SYSTEM_IN_MAINTENANCE, SYSTEM_UPDATING, EMERGENCY_LOCK_ACTIVE,
    CONFIRMATION_REQUIRED, INVALID_CONFIRM_TOKEN, CONFIRM_TOKEN_EXPIRED,
    UNAUTHORIZED, PERMISSION_DENIED, TARGET_NOT_FOUND, TARGET_DISABLED, TARGET_OFFLINE,
    COMMAND_NOT_SUPPORTED, SAFETY_INTERLOCK, EXECUTION_TIMEOUT, HARDWARE_FAILURE,
    COMMUNICATION_ERROR, QUEUE_FULL, CANCELLED
};
inline bool isValidCommandErrorCode(CommandErrorCode value)
{ return static_cast<uint16_t>(value)<=static_cast<uint16_t>(CommandErrorCode::CANCELLED); }

namespace CommandText
{
inline bool isTerminated(const char* value,size_t capacity)
{ if(value==nullptr) return false; for(size_t i=0;i<capacity;++i) if(value[i]=='\0') return true; return false; }
inline bool isCanonical(const char* value,size_t capacity)
{
    if(value==nullptr) return false;
    bool terminated=false;
    for(size_t i=0;i<capacity;++i)
    {
        if(value[i]=='\0') terminated=true;
        else if(terminated) return false;
    }
    return terminated;
}
inline bool set(char* destination,size_t capacity,const char* value)
{
    if(value==nullptr) return false;
    size_t length=0; while(length<capacity&&value[length]!='\0') ++length;
    if(length>=capacity) return false;
    for(size_t i=0;i<capacity;++i) destination[i]='\0';
    for(size_t i=0;i<length;++i) destination[i]=value[i];
    return true;
}
}

#endif
