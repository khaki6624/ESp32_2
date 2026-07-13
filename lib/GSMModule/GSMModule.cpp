#include "GSMModule.h"

#include <stdio.h>
#include <string.h>

GSMModule::GSMModule(
    HardwareSerial& uart,
    int8_t rx,
    int8_t tx,
    uint32_t baud
) :
    serial(uart),
    rxPin(rx),
    txPin(tx),
    baudRate(baud),
    state(GSMState::OFF),
    networkStatus(GSMNetworkStatus::UNKNOWN),
    callState(GSMCallState::IDLE),
    signal{},
    smsStorageStatus{},
    operatorName{},
    beginRequested(false),
    resetRequested(false),
    commandActive(false),
    waitingForPrompt(false),
    smsHeaderReceived(false),
    simReady(false),
    atReady(false),
    echoDisabled(false),
    stateStartedMs(0),
    commandStartedMs(0),
    lastNetworkRequestMs(0),
    lastATRequestMs(0),
    commandQueue{},
    commandHead(0),
    commandTail(0),
    commandCount(0),
    activeCommand{},
    smsQueue{},
    smsHead(0),
    smsTail(0),
    smsCount(0),
    incomingSMS{},
    pendingSMSIndexes{},
    pendingSMSCount(0),
    smsQueueOverflow(false),
    pendingCall{},
    pendingCallAvailable(false),
    lineBuffer{},
    lineLength(0)
{
}

void GSMModule::begin()
{
    serial.begin(baudRate, SERIAL_8N1, rxPin, txPin);
    beginRequested = true;
}

void GSMModule::update()
{
    readSerial();

    if (resetRequested)
        performReset();

    processStateMachine();

    retryPendingSMS();

    if (commandActive && millis() - commandStartedMs >= COMMAND_TIMEOUT_MS)
    {
        finishCommand(false);
    }

    if (!commandActive)
        startNextCommand();
}

void GSMModule::reset()
{
    resetRequested = true;
}

void GSMModule::performReset()
{
    // فقط State داخلی Driver پاک می‌شود؛ Reset سخت‌افزاری SIM800C انجام نمی‌شود.
    resetRequested = false;
    beginRequested = true;
    clearCommandQueue();
    commandActive = false;
    waitingForPrompt = false;
    memset(lineBuffer, 0, sizeof(lineBuffer));
    lineLength = 0;
    networkStatus = GSMNetworkStatus::UNKNOWN;
    signal = GSMSignal{};
    smsStorageStatus = SMSStorageStatus{};
    memset(operatorName, 0, sizeof(operatorName));
    callState = GSMCallState::IDLE;
    simReady = false;
    atReady = false;
    echoDisabled = false;
    clearSMS();
    incomingSMS = SMSMessage{};
    pendingCall = GSMCallInfo{};
    pendingCallAvailable = false;
    smsHeaderReceived = false;
    memset(pendingSMSIndexes, 0, sizeof(pendingSMSIndexes));
    pendingSMSCount = 0;
    smsQueueOverflow = false;
    activeCommand = ATCommand{};
    commandStartedMs = 0;
    lastNetworkRequestMs = 0;
    lastATRequestMs = 0;
    setState(GSMState::OFF);
}

bool GSMModule::isReady() const
{
    return state == GSMState::READY;
}

bool GSMModule::isBusy() const
{
    return commandActive || commandCount > 0 || waitingForPrompt;
}

GSMState GSMModule::getState() const
{
    return state;
}

bool GSMModule::smsAvailable() const
{
    return smsCount > 0;
}

const SMSMessage& GSMModule::peekSMS() const
{
    static const SMSMessage emptyMessage{};
    return smsCount > 0 ? smsQueue[smsHead] : emptyMessage;
}

SMSMessage GSMModule::readSMS()
{
    if (smsCount == 0)
        return SMSMessage{};

    SMSMessage message = smsQueue[smsHead];
    smsQueue[smsHead] = SMSMessage{};
    smsHead = (smsHead + 1) % SMS_QUEUE_CAPACITY;
    --smsCount;
    return message;
}

void GSMModule::clearSMS()
{
    for (uint8_t index = 0; index < SMS_QUEUE_CAPACITY; ++index)
        smsQueue[index] = SMSMessage{};

    smsHead = 0;
    smsTail = 0;
    smsCount = 0;
}

bool GSMModule::sendSMS(const char* phoneNumber, const char* text)
{
    if (!isReady() || phoneNumber == nullptr || phoneNumber[0] == '\0' ||
        text == nullptr || text[0] == '\0' ||
        strlen(phoneNumber) >= GSM_PHONE_NUMBER_LENGTH ||
        strlen(text) >= GSM_SMS_TEXT_LENGTH)
    {
        return false;
    }

    char command[COMMAND_LENGTH];
    const int written = snprintf(command, sizeof(command), "AT+CMGS=\"%s\"", phoneNumber);
    if (written < 0 || static_cast<size_t>(written) >= sizeof(command))
        return false;

    return enqueueCommand(CommandType::SEND_SMS, command, text);
}

bool GSMModule::deleteSMS(uint16_t storageIndex)
{
    if (!isReady() || storageIndex == 0)
        return false;

    char command[32];
    snprintf(command, sizeof(command), "AT+CMGD=%u", storageIndex);
    return enqueueCommand(CommandType::DELETE_SMS, command, nullptr, storageIndex);
}

bool GSMModule::deleteAllSMS()
{
    return isReady() && enqueueCommand(CommandType::DELETE_SMS, "AT+CMGD=1,4");
}

bool GSMModule::call(const char* phoneNumber)
{
    if (!isReady() || phoneNumber == nullptr || phoneNumber[0] == '\0' ||
        strlen(phoneNumber) >= GSM_PHONE_NUMBER_LENGTH)
    {
        return false;
    }

    char command[COMMAND_LENGTH];
    const int written = snprintf(command, sizeof(command), "ATD%s;", phoneNumber);
    if (written < 0 || static_cast<size_t>(written) >= sizeof(command))
        return false;

    if (!enqueueCommand(CommandType::DIAL, command))
        return false;

    storeCall(false, false, phoneNumber);
    callState = GSMCallState::DIALING;
    return true;
}

bool GSMModule::answer()
{
    return isReady() && enqueueCommand(CommandType::ANSWER, "ATA");
}

bool GSMModule::hangup()
{
    return isReady() && enqueueCommand(CommandType::HANGUP, "ATH");
}

bool GSMModule::callAvailable() const
{
    return pendingCallAvailable;
}

const GSMCallInfo& GSMModule::peekCall() const
{
    static const GSMCallInfo emptyCall{};
    return pendingCallAvailable ? pendingCall : emptyCall;
}

GSMCallInfo GSMModule::readCall()
{
    if (!pendingCallAvailable)
        return GSMCallInfo{};

    GSMCallInfo callInfo = pendingCall;
    pendingCall = GSMCallInfo{};
    pendingCallAvailable = false;
    return callInfo;
}

GSMCallState GSMModule::getCallState() const
{
    return callState;
}

bool GSMModule::requestSignalQuality()
{
    return isReady() && enqueueCommand(CommandType::SIGNAL_QUALITY, "AT+CSQ");
}

bool GSMModule::requestNetworkStatus()
{
    if (state != GSMState::READY ||
        commandCount > COMMAND_QUEUE_CAPACITY - 2)
    {
        return false;
    }

    // وضعیت ثبت شبکه و نام اپراتور در یک درخواست غیرمسدودکننده صف می‌شوند.
    return enqueueCommandPair(
        CommandType::NETWORK_STATUS,
        "AT+CREG?",
        CommandType::OPERATOR_STATUS,
        "AT+COPS?"
    );
}

bool GSMModule::requestSMSStorageStatus()
{
    return isReady() && enqueueCommand(CommandType::SMS_STORAGE_STATUS, "AT+CPMS?");
}

const char* GSMModule::getOperator() const
{
    // اشاره‌گر فقط به Buffer داخلی و با مالکیت خود GSMModule است.
    return operatorName;
}

const GSMSignal& GSMModule::getSignal() const
{
    return signal;
}

int8_t GSMModule::getSignalQuality() const
{
    // فقط برای سازگاری API قدیمی؛ مقدار RSSI را برمی‌گرداند.
    return signal.rssi;
}

GSMNetworkStatus GSMModule::getNetworkStatus() const
{
    return networkStatus;
}

const SMSStorageStatus& GSMModule::getSMSStorageStatus() const
{
    return smsStorageStatus;
}

bool GSMModule::isSMSStorageFull() const
{
    // فقط وضعیت را گزارش می‌کند و هیچ Cleanup انجام نمی‌دهد.
    return smsStorageStatus.valid && smsStorageStatus.total > 0 &&
           smsStorageStatus.used >= smsStorageStatus.total;
}

bool GSMModule::hasSMSQueueOverflow() const
{
    return smsQueueOverflow;
}

void GSMModule::clearSMSQueueOverflow()
{
    smsQueueOverflow = false;
}

bool GSMModule::enqueueCommand(
    CommandType type,
    const char* command,
    const char* payload,
    uint16_t storageIndex
)
{
    if (command == nullptr || command[0] == '\0' ||
        strlen(command) >= COMMAND_LENGTH || commandCount >= COMMAND_QUEUE_CAPACITY)
    {
        return false;
    }

    ATCommand& queued = commandQueue[commandTail];
    queued = ATCommand{};
    queued.type = type;
    queued.storageIndex = storageIndex;
    copyText(queued.command, sizeof(queued.command), command);
    copyText(queued.payload, sizeof(queued.payload), payload);

    commandTail = (commandTail + 1) % COMMAND_QUEUE_CAPACITY;
    ++commandCount;
    return true;
}

bool GSMModule::enqueueCommandPair(
    CommandType firstType,
    const char* firstCommand,
    CommandType secondType,
    const char* secondCommand
)
{
    // فرمان‌های چندتایی Initialization باید یا کامل وارد Queue شوند یا اصلاً نشوند.
    if (commandCount > COMMAND_QUEUE_CAPACITY - 2)
        return false;

    const uint8_t originalTail = commandTail;
    const uint8_t originalCount = commandCount;
    if (enqueueCommand(firstType, firstCommand) &&
        enqueueCommand(secondType, secondCommand))
    {
        return true;
    }

    while (commandCount > originalCount)
    {
        commandTail = (commandTail + COMMAND_QUEUE_CAPACITY - 1) % COMMAND_QUEUE_CAPACITY;
        commandQueue[commandTail] = ATCommand{};
        --commandCount;
    }
    commandTail = originalTail;
    return false;
}

void GSMModule::clearCommandQueue()
{
    for (uint8_t index = 0; index < COMMAND_QUEUE_CAPACITY; ++index)
        commandQueue[index] = ATCommand{};

    commandHead = 0;
    commandTail = 0;
    commandCount = 0;
}

void GSMModule::startNextCommand()
{
    if (commandCount == 0)
        return;

    activeCommand = commandQueue[commandHead];
    commandQueue[commandHead] = ATCommand{};
    commandHead = (commandHead + 1) % COMMAND_QUEUE_CAPACITY;
    --commandCount;

    serial.print(activeCommand.command);
    serial.print("\r\n");
    commandActive = true;
    waitingForPrompt = activeCommand.type == CommandType::SEND_SMS;
    smsHeaderReceived = false;
    commandStartedMs = millis();

    if (activeCommand.type == CommandType::READ_SMS)
    {
        incomingSMS = SMSMessage{};
        incomingSMS.storageIndex = activeCommand.storageIndex;
    }
}

void GSMModule::finishCommand(bool success)
{
    const CommandType completedType = activeCommand.type;
    commandActive = false;
    waitingForPrompt = false;
    activeCommand = ATCommand{};

    if (!success)
    {
        // خطای موقت AT یا شبکه نباید Driver را وارد ERROR دائمی کند.
        if (state == GSMState::WAIT_AT &&
            (completedType == CommandType::AT || completedType == CommandType::ECHO_OFF))
        {
            return;
        }
        if (state == GSMState::WAIT_NETWORK &&
            completedType == CommandType::NETWORK_STATUS)
        {
            return;
        }
        if (state != GSMState::READY)
            setState(GSMState::ERROR);
        return;
    }

    if (completedType == CommandType::AT)
        atReady = true;
    else if (completedType == CommandType::ECHO_OFF)
        echoDisabled = true;

    if (completedType == CommandType::SIM_STATUS && !simReady)
    {
        setState(GSMState::ERROR);
        return;
    }

    if (completedType == CommandType::DIAL)
        callState = GSMCallState::DIALING;
    else if (completedType == CommandType::SMS_TEXT_MODE &&
             state == GSMState::WAIT_NETWORK)
    {
        setState(GSMState::READY);
    }
    else if (completedType == CommandType::ANSWER)
    {
        callState = GSMCallState::ACTIVE;
        pendingCall.active = true;
    }
    else if (completedType == CommandType::HANGUP)
    {
        callState = GSMCallState::ENDED;
        pendingCall.active = false;
    }
}

void GSMModule::readSerial()
{
    while (serial.available() > 0)
    {
        const char character = static_cast<char>(serial.read());

        if (waitingForPrompt && character == '>')
        {
            handlePrompt();
            continue;
        }

        if (character == '\r')
            continue;

        if (character == '\n')
        {
            if (lineLength > 0)
            {
                lineBuffer[lineLength] = '\0';
                processLine(lineBuffer);
                lineLength = 0;
            }
            continue;
        }

        if (lineLength < LINE_LENGTH - 1)
            lineBuffer[lineLength++] = character;
        else
            lineLength = 0;
    }
}

void GSMModule::processLine(char* line)
{
    if (line == nullptr || line[0] == '\0')
        return;

    processURC(line);
    if (commandActive)
        processCommandResponse(line);
}

void GSMModule::processURC(const char* line)
{
    if (strncmp(line, "+CMTI:", 6) == 0)
    {
        const char* comma = strrchr(line, ',');
        if (comma != nullptr)
        {
            const uint16_t index = static_cast<uint16_t>(atoi(comma + 1));
            if (index > 0)
            {
                if (smsCount >= SMS_QUEUE_CAPACITY || pendingSMSCount > 0)
                {
                    smsQueueOverflow = true;
                    rememberPendingSMS(index);
                    return;
                }
                char command[32];
                snprintf(command, sizeof(command), "AT+CMGR=%u", index);
                enqueueCommand(CommandType::READ_SMS, command, nullptr, index);
            }
        }
    }
    else if (strcmp(line, "RING") == 0)
    {
        storeCall(true, false);
        callState = GSMCallState::INCOMING;
    }
    else if (strcmp(line, "NO CARRIER") == 0)
    {
        callState = GSMCallState::ENDED;
        pendingCall.active = false;
    }
    else if (strncmp(line, "+CLIP:", 6) == 0)
    {
        char caller[GSM_PHONE_NUMBER_LENGTH];
        if (extractQuotedField(line, 0, caller, sizeof(caller)))
            storeCall(true, false, caller);
    }
    else if (strncmp(line, "+CREG:", 6) == 0)
    {
        if (!commandActive || activeCommand.type != CommandType::NETWORK_STATUS)
            parseNetworkStatus(line);
    }

    // Driver درباره Whitelist یا حذف پیام تبلیغاتی تصمیمی نمی‌گیرد.
}

void GSMModule::processCommandResponse(const char* line)
{
    if (activeCommand.type == CommandType::NETWORK_STATUS &&
        strncmp(line, "+CREG:", 6) == 0)
        parseNetworkStatus(line);
    else if (activeCommand.type == CommandType::SIGNAL_QUALITY &&
             strncmp(line, "+CSQ:", 5) == 0)
        parseSignalQuality(line);
    else if (activeCommand.type == CommandType::OPERATOR_STATUS &&
             strncmp(line, "+COPS:", 6) == 0)
        parseOperator(line);
    else if (activeCommand.type == CommandType::SMS_STORAGE_STATUS &&
             strncmp(line, "+CPMS:", 6) == 0)
        parseSMSStorageStatus(line);
    else if (activeCommand.type == CommandType::SIM_STATUS &&
             strcmp(line, "+CPIN: READY") == 0)
    {
        simReady = true;
    }
    else if (activeCommand.type == CommandType::READ_SMS &&
             strncmp(line, "+CMGR:", 6) == 0)
    {
        parseSMSHeader(line);
    }
    else if (activeCommand.type == CommandType::READ_SMS && smsHeaderReceived &&
             strcmp(line, "OK") != 0 && strcmp(line, "ERROR") != 0)
    {
        appendSMSLine(line);
    }

    if (strcmp(line, "OK") == 0)
    {
        if (activeCommand.type == CommandType::READ_SMS)
            storeIncomingSMS();
        finishCommand(true);
    }
    else if (strcmp(line, "ERROR") == 0 || strncmp(line, "+CME ERROR:", 11) == 0 ||
             strncmp(line, "+CMS ERROR:", 11) == 0)
    {
        finishCommand(false);
    }
}

void GSMModule::processStateMachine()
{
    const uint32_t now = millis();

    if (state == GSMState::OFF && beginRequested)
    {
        beginRequested = false;
        setState(GSMState::BOOTING);
        return;
    }

    if (state == GSMState::BOOTING && now - stateStartedMs >= BOOT_DELAY_MS)
    {
        setState(GSMState::WAIT_AT);
        requestInitializationCommand();
        return;
    }

    if (state == GSMState::WAIT_AT && !commandActive && commandCount == 0 &&
        atReady && echoDisabled)
    {
        // مرحله WAIT_AT فقط پس از موفقیت AT و ATE0 کامل می‌شود.
        setState(GSMState::WAIT_SIM);
        requestInitializationCommand();
        return;
    }


    if (state == GSMState::WAIT_AT && !commandActive && commandCount == 0 &&
        (!atReady || !echoDisabled) && now - lastATRequestMs >= AT_RETRY_MS)
    {
        requestInitializationCommand();
        return;
    }

    if (state == GSMState::WAIT_SIM && !commandActive && commandCount == 0 && simReady)
    {
        setState(GSMState::WAIT_NETWORK);
        requestInitializationCommand();
        return;
    }

    if (state == GSMState::WAIT_NETWORK && !commandActive && commandCount == 0)
    {
        if (networkStatus == GSMNetworkStatus::REGISTERED_HOME ||
            networkStatus == GSMNetworkStatus::REGISTERED_ROAMING)
        {
            enqueueCommand(CommandType::SMS_TEXT_MODE, "AT+CMGF=1");
        }
        else if (now - lastNetworkRequestMs >= NETWORK_RETRY_MS)
        {
            requestInitializationCommand();
        }
    }
}

void GSMModule::requestInitializationCommand()
{
    switch (state)
    {
        case GSMState::WAIT_AT:
            if (enqueueCommandPair(
                    CommandType::AT,
                    "AT",
                    CommandType::ECHO_OFF,
                    "ATE0"
                ))
            {
                lastATRequestMs = millis();
            }
            break;

        case GSMState::WAIT_SIM:
            enqueueCommand(CommandType::SIM_STATUS, "AT+CPIN?");
            break;

        case GSMState::WAIT_NETWORK:
            if (enqueueCommand(CommandType::NETWORK_STATUS, "AT+CREG?"))
                lastNetworkRequestMs = millis();
            break;

        default:
            break;
    }
}

void GSMModule::setState(GSMState newState)
{
    state = newState;
    stateStartedMs = millis();
}

void GSMModule::handlePrompt()
{
    serial.print(activeCommand.payload);
    serial.write(static_cast<uint8_t>(26));
    waitingForPrompt = false;
    commandStartedMs = millis();
}

void GSMModule::parseNetworkStatus(const char* line)
{
    const char* comma = strrchr(line, ',');
    const int status = atoi(comma != nullptr ? comma + 1 : line + 6);

    switch (status)
    {
        case 0: networkStatus = GSMNetworkStatus::NOT_REGISTERED; break;
        case 1: networkStatus = GSMNetworkStatus::REGISTERED_HOME; break;
        case 2: networkStatus = GSMNetworkStatus::SEARCHING; break;
        case 3: networkStatus = GSMNetworkStatus::REGISTRATION_DENIED; break;
        case 5: networkStatus = GSMNetworkStatus::REGISTERED_ROAMING; break;
        default: networkStatus = GSMNetworkStatus::UNKNOWN; break;
    }
}

void GSMModule::parseSignalQuality(const char* line)
{
    signal = GSMSignal{};
    int rssi = -1;
    int ber = -1;
    if (line == nullptr || sscanf(line, "+CSQ: %d,%d", &rssi, &ber) != 2)
        return;

    GSMSignal parsed;
    parsed.rssi = rssi >= 0 && rssi <= 31 ? static_cast<int8_t>(rssi) : -1;
    parsed.ber = ber >= 0 && ber <= 7 ? static_cast<int8_t>(ber) : -1;
    parsed.valid = true;
    signal = parsed;
}

void GSMModule::parseOperator(const char* line)
{
    char parsed[GSM_OPERATOR_NAME_LENGTH] = {};
    if (line != nullptr && strncmp(line, "+COPS:", 6) == 0 &&
        extractQuotedField(line, 0, parsed, sizeof(parsed)))
    {
        copyText(operatorName, sizeof(operatorName), parsed);
    }
}

void GSMModule::parseSMSStorageStatus(const char* line)
{
    smsStorageStatus = SMSStorageStatus{};
    unsigned int used = 0;
    unsigned int total = 0;
    if (line == nullptr ||
        sscanf(line, "+CPMS: \"%*[^\"]\",%u,%u", &used, &total) != 2 ||
        used > UINT16_MAX || total > UINT16_MAX)
    {
        return;
    }

    SMSStorageStatus parsed;
    parsed.used = static_cast<uint16_t>(used);
    parsed.total = static_cast<uint16_t>(total);
    parsed.valid = true;
    smsStorageStatus = parsed;
}

void GSMModule::parseSMSHeader(const char* line)
{
    incomingSMS.valid = false;
    extractQuotedField(line, 1, incomingSMS.sender, sizeof(incomingSMS.sender));
    extractQuotedField(line, 3, incomingSMS.timestamp, sizeof(incomingSMS.timestamp));
    smsHeaderReceived = true;
}

void GSMModule::appendSMSLine(const char* line)
{
    if (line == nullptr)
        return;

    const size_t currentLength = strlen(incomingSMS.text);
    const size_t lineLengthValue = strlen(line);
    const size_t separatorLength = currentLength > 0 ? 1 : 0;

    if (currentLength + separatorLength + lineLengthValue >= sizeof(incomingSMS.text))
        return;

    if (separatorLength > 0)
        incomingSMS.text[currentLength] = '\n';

    memcpy(
        incomingSMS.text + currentLength + separatorLength,
        line,
        lineLengthValue + 1
    );
}

void GSMModule::storeIncomingSMS()
{
    if (incomingSMS.storageIndex == 0 || incomingSMS.sender[0] == '\0')
        return;

    incomingSMS.valid = true;

    // صف پر نباید هیچ پیام معتبر قبلی را برای پیام جدید حذف کند.
    if (smsCount == SMS_QUEUE_CAPACITY)
    {
        smsQueueOverflow = true;
        rememberPendingSMS(incomingSMS.storageIndex);
        incomingSMS = SMSMessage{};
        return;
    }

    smsQueue[smsTail] = incomingSMS;
    smsTail = (smsTail + 1) % SMS_QUEUE_CAPACITY;
    ++smsCount;
    forgetPendingSMS(incomingSMS.storageIndex);
    incomingSMS = SMSMessage{};
}

void GSMModule::rememberPendingSMS(uint16_t storageIndex)
{
    for (uint8_t index = 0; index < pendingSMSCount; ++index)
    {
        if (pendingSMSIndexes[index] == storageIndex)
            return;
    }

    if (storageIndex > 0 && pendingSMSCount < PENDING_SMS_CAPACITY)
        pendingSMSIndexes[pendingSMSCount++] = storageIndex;
}

void GSMModule::forgetPendingSMS(uint16_t storageIndex)
{
    for (uint8_t index = 0; index < pendingSMSCount; ++index)
    {
        if (pendingSMSIndexes[index] != storageIndex)
            continue;

        for (uint8_t move = index + 1; move < pendingSMSCount; ++move)
            pendingSMSIndexes[move - 1] = pendingSMSIndexes[move];
        pendingSMSIndexes[--pendingSMSCount] = 0;
        return;
    }
}

bool GSMModule::hasQueuedSMSRead() const
{
    if (commandActive && activeCommand.type == CommandType::READ_SMS)
        return true;

    for (uint8_t offset = 0; offset < commandCount; ++offset)
    {
        const uint8_t index = (commandHead + offset) % COMMAND_QUEUE_CAPACITY;
        if (commandQueue[index].type == CommandType::READ_SMS)
            return true;
    }
    return false;
}

void GSMModule::retryPendingSMS()
{
    if (!isReady() || pendingSMSCount == 0 || smsCount >= SMS_QUEUE_CAPACITY ||
        hasQueuedSMSRead())
    {
        return;
    }

    char command[32];
    const uint16_t storageIndex = pendingSMSIndexes[0];
    const int written = snprintf(command, sizeof(command), "AT+CMGR=%u", storageIndex);
    if (written < 0 || static_cast<size_t>(written) >= sizeof(command) ||
        !enqueueCommand(CommandType::READ_SMS, command, nullptr, storageIndex))
    {
        return;
    }

}

void GSMModule::storeCall(bool incoming, bool active, const char* caller)
{
    if (caller != nullptr)
        copyText(pendingCall.caller, sizeof(pendingCall.caller), caller);

    pendingCall.incoming = incoming;
    pendingCall.active = active;
    pendingCall.timestampMs = millis();
    pendingCallAvailable = true;
}

void GSMModule::copyText(char* destination, size_t capacity, const char* source)
{
    if (destination == nullptr || capacity == 0)
        return;

    if (source == nullptr)
    {
        destination[0] = '\0';
        return;
    }

    strncpy(destination, source, capacity - 1);
    destination[capacity - 1] = '\0';
}

bool GSMModule::extractQuotedField(
    const char* source,
    uint8_t fieldIndex,
    char* destination,
    size_t capacity
)
{
    if (source == nullptr || destination == nullptr || capacity == 0)
        return false;

    const char* cursor = source;
    for (uint8_t index = 0; index <= fieldIndex; ++index)
    {
        cursor = strchr(cursor, '\"');
        if (cursor == nullptr)
            return false;
        ++cursor;

        const char* end = strchr(cursor, '\"');
        if (end == nullptr)
            return false;

        if (index == fieldIndex)
        {
            size_t length = static_cast<size_t>(end - cursor);
            if (length >= capacity)
                length = capacity - 1;
            memcpy(destination, cursor, length);
            destination[length] = '\0';
            return true;
        }

        cursor = end + 1;
    }

    return false;
}
