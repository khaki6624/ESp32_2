#ifndef GSM_MODULE_H
#define GSM_MODULE_H

#include <Arduino.h>
#include <GSMCommon.h>

class GSMModule
{
private:
    static constexpr uint8_t COMMAND_QUEUE_CAPACITY = 8;
    static constexpr uint8_t SMS_QUEUE_CAPACITY = 4;
    static constexpr uint16_t COMMAND_LENGTH = 192;
    static constexpr uint16_t LINE_LENGTH = 192;
    static constexpr uint32_t BOOT_DELAY_MS = 3000;
    static constexpr uint32_t COMMAND_TIMEOUT_MS = 10000;
    static constexpr uint32_t NETWORK_RETRY_MS = 3000;

    enum class CommandType : uint8_t
    {
        GENERIC = 0,
        AT,
        ECHO_OFF,
        SIM_STATUS,
        NETWORK_STATUS,
        SIGNAL_QUALITY,
        OPERATOR_STATUS,
        SMS_TEXT_MODE,
        READ_SMS,
        DELETE_SMS,
        SEND_SMS,
        DIAL,
        ANSWER,
        HANGUP
    };

    struct ATCommand
    {
        CommandType type = CommandType::GENERIC;
        char command[COMMAND_LENGTH] = {};
        char payload[GSM_SMS_TEXT_LENGTH] = {};
        uint16_t storageIndex = 0;
    };

    HardwareSerial& serial;
    int8_t rxPin;
    int8_t txPin;
    uint32_t baudRate;

    GSMState state;
    GSMNetworkStatus networkStatus;
    GSMCallState callState;
    int8_t signalQuality;

    bool beginRequested;
    bool resetRequested;
    bool commandActive;
    bool waitingForPrompt;
    bool smsHeaderReceived;
    bool simReady;
    uint32_t stateStartedMs;
    uint32_t commandStartedMs;
    uint32_t lastNetworkRequestMs;

    ATCommand commandQueue[COMMAND_QUEUE_CAPACITY];
    uint8_t commandHead;
    uint8_t commandTail;
    uint8_t commandCount;
    ATCommand activeCommand;

    SMSMessage smsQueue[SMS_QUEUE_CAPACITY];
    uint8_t smsHead;
    uint8_t smsTail;
    uint8_t smsCount;
    SMSMessage incomingSMS;

    GSMCallInfo pendingCall;
    bool pendingCallAvailable;

    char lineBuffer[LINE_LENGTH];
    uint16_t lineLength;

    bool enqueueCommand(
        CommandType type,
        const char* command,
        const char* payload = nullptr,
        uint16_t storageIndex = 0
    );
    void clearCommandQueue();
    void startNextCommand();
    void finishCommand(bool success);
    void readSerial();
    void processLine(char* line);
    void processURC(const char* line);
    void processCommandResponse(const char* line);
    void processStateMachine();
    void requestInitializationCommand();
    void setState(GSMState newState);
    void handlePrompt();
    void parseNetworkStatus(const char* line);
    void parseSignalQuality(const char* line);
    void parseSMSHeader(const char* line);
    void appendSMSLine(const char* line);
    void storeIncomingSMS();
    void storeCall(bool incoming, bool active, const char* caller = nullptr);

    static void copyText(char* destination, size_t capacity, const char* source);
    static bool extractQuotedField(
        const char* source,
        uint8_t fieldIndex,
        char* destination,
        size_t capacity
    );

public:
    GSMModule(
        HardwareSerial& uart,
        int8_t rx,
        int8_t tx,
        uint32_t baud = 9600
    );

    void begin();
    void update();
    void reset();

    bool isReady() const;
    bool isBusy() const;
    GSMState getState() const;

    bool smsAvailable() const;
    const SMSMessage& peekSMS() const;
    SMSMessage readSMS();
    void clearSMS();
    bool sendSMS(const char* phoneNumber, const char* text);
    bool deleteSMS(uint16_t storageIndex);
    bool deleteAllSMS();

    bool call(const char* phoneNumber);
    bool answer();
    bool hangup();
    bool callAvailable() const;
    const GSMCallInfo& peekCall() const;
    GSMCallInfo readCall();
    GSMCallState getCallState() const;

    bool requestSignalQuality();
    bool requestNetworkStatus();
    int8_t getSignalQuality() const;
    GSMNetworkStatus getNetworkStatus() const;
};

#endif
