#ifndef GSM_COMMON_H
#define GSM_COMMON_H

#include <Arduino.h>

constexpr uint8_t GSM_PHONE_NUMBER_LENGTH = 24;
constexpr uint16_t GSM_SMS_TEXT_LENGTH = 161;
constexpr uint8_t GSM_TIMESTAMP_LENGTH = 24;
constexpr uint8_t GSM_OPERATOR_NAME_LENGTH = 32;

struct GSMSignal
{
    int8_t rssi = -1;
    int8_t ber = -1;
    bool valid = false;
};

struct SMSStorageStatus
{
    uint16_t used = 0;
    uint16_t total = 0;
    bool valid = false;
};

enum class GSMState : uint8_t
{
    OFF = 0,
    BOOTING,
    WAIT_AT,
    WAIT_SIM,
    WAIT_NETWORK,
    READY,
    ERROR
};

enum class GSMNetworkStatus : uint8_t
{
    UNKNOWN = 0,
    NOT_REGISTERED,
    SEARCHING,
    REGISTERED_HOME,
    REGISTRATION_DENIED,
    REGISTERED_ROAMING
};

enum class GSMCallState : uint8_t
{
    IDLE = 0,
    INCOMING,
    DIALING,
    ACTIVE,
    ENDED
};

struct SMSMessage
{
    uint16_t storageIndex = 0;
    char sender[GSM_PHONE_NUMBER_LENGTH] = {};
    char text[GSM_SMS_TEXT_LENGTH] = {};
    char timestamp[GSM_TIMESTAMP_LENGTH] = {};
    bool valid = false;
};

struct GSMCallInfo
{
    char caller[GSM_PHONE_NUMBER_LENGTH] = {};
    bool incoming = false;
    bool active = false;
    uint32_t timestampMs = 0;
};

#endif
