#include "DelsamCompositionRoot.h"

#include <CommandDispatcher.h>
#include <CommandParser.h>
#include <CommandValidator.h>
#include <DeviceCommandHandler.h>
#include <DeviceCommandRouteHandler.h>
#include <DeviceQueryHandler.h>
#include <DeviceRegistry.h>
#include <DeviceTemplateRegistry.h>
#include <InputManager.h>
#include <OutputManager.h>
#include <StaticDriverBindingResolver.h>
#include <AnalogInputAdapter.h>
#include <DigitalInputAdapter.h>
#include <RelayOutputAdapter.h>
#include <BoardConfig.h>
#include <IRReceiver.h>
#include <IRSender.h>
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#ifdef SERIAL
#undef SERIAL
#endif

namespace
{
constexpr NodeId LOCAL_NODE_ID = 1U;
constexpr LocationId LOCAL_LOCATION_ID = 1U;
constexpr size_t SERIAL_LINE_CAPACITY = COMMAND_ORIGINAL_TEXT_MAX_LENGTH;
constexpr size_t SERIAL_BYTES_PER_UPDATE = 32U;

struct RelaySlot
{
    Relay driver;
    RelayOutputAdapter adapter;
    RelaySlot(uint8_t pin, bool activeLow) : driver(pin, activeLow), adapter(driver) {}
};

RelaySlot relaySlots[] = {
    {23U,true},{22U,true},{21U,true},{19U,true},{18U,true},{17U,true},{16U,true},{15U,true},
    {14U,true},{13U,true},{12U,true},{27U,true},{26U,true},
    // GPIO25 is reserved for the Service Button and is intentionally not instantiated.
    {33U,true},{32U,true}
};
constexpr size_t ACTIVE_RELAY_COUNT = sizeof(relaySlots) / sizeof(relaySlots[0]);

DigitalInput digitalInput(
    BoardConfig::inputs[0].gpio,
    BoardConfig::inputs[0].activeLow,
    BoardConfig::inputs[0].usePullup,
    BoardConfig::inputs[0].debounceMs);
DigitalInputAdapter digitalInputAdapter(digitalInput);
AnalogInput analogInput(
    BoardConfig::analogInputs[0].gpio,
    BoardConfig::analogInputs[0].referenceVoltage,
    BoardConfig::analogInputs[0].adcMax,
    BoardConfig::analogInputs[0].filterAlpha,
    BoardConfig::analogInputs[0].threshold);
AnalogInputAdapter analogInputAdapter(analogInput);
IRReceiver irReceiver(BoardConfig::IR_RECEIVER_PIN);
IRSender irSender(BoardConfig::IR_SENDER_PIN);

DeviceTemplateRegistry templateRegistry;
DeviceRegistry deviceRegistry(templateRegistry);
StaticDriverBindingResolver resolver(LOCAL_NODE_ID);
OutputManager outputManager(resolver);
InputManager inputManager(resolver);
DeviceCommandHandler deviceCommandHandler(deviceRegistry, templateRegistry, outputManager);
DeviceQueryHandler deviceQueryHandler(
    deviceRegistry, templateRegistry, outputManager, inputManager);
DeviceCommandRouteHandler deviceRouteHandler(deviceCommandHandler, deviceQueryHandler);
CommandDispatcher commandDispatcher;
CommandParser commandParser;
CommandValidator commandValidator;

char serialLine[SERIAL_LINE_CAPACITY] = {};
size_t serialLength = 0U;
bool serialOverflow = false;
bool previousWasCr = false;
bool ready = false;
CommandId nextCommandId = 1U;
RequestId nextRequestId = 1U;

DeviceBinding makeBinding(DriverType type, uint8_t channel)
{
    DeviceBinding binding;
    binding.nodeId = LOCAL_NODE_ID;
    binding.driverType = type;
    binding.channel = channel;
    binding.valid = true;
    return binding;
}

DeviceTemplate makeTemplate(
    DeviceTemplateId id,
    const char* name,
    DriverType type,
    DeviceValueType valueType,
    bool output)
{
    DeviceTemplate value;
    value.id = id;
    value.setName(name);
    value.valueType = valueType;
    addDriverType(value.allowedDriverTypes, type);
    if (output)
    {
        addAction(value.allowedActions, DeviceAction::ON);
        addAction(value.allowedActions, DeviceAction::OFF);
        addAction(value.allowedActions, DeviceAction::TOGGLE);
        addAction(value.allowedActions, DeviceAction::PULSE);
    }
    value.systemTemplate = true;
    value.enabled = true;
    return value;
}

Device makeDevice(
    DeviceId id,
    DeviceTemplateId templateId,
    const char* name,
    const DeviceBinding& binding)
{
    Device device;
    device.id = id;
    device.templateId = templateId;
    device.setName(name);
    device.locationId = LOCAL_LOCATION_ID;
    device.binding = binding;
    device.enabled = true;
    device.configured = true;
    device.health = DeviceHealth::OK;
    return device;
}

bool configureDevices()
{
    if (!templateRegistry.add(makeTemplate(
            1U, "relay", DriverType::RELAY, DeviceValueType::BOOLEAN, true)) ||
        !templateRegistry.add(makeTemplate(
            2U, "digital-input", DriverType::DIGITAL_INPUT, DeviceValueType::BOOLEAN, false)) ||
        !templateRegistry.add(makeTemplate(
            3U, "analog-input", DriverType::ANALOG_INPUT, DeviceValueType::INTEGER, false)))
        return false;

    for (size_t index = 0U; index < ACTIVE_RELAY_COUNT; ++index)
    {
        const uint8_t channel = static_cast<uint8_t>(index + 1U);
        const DeviceBinding binding = makeBinding(DriverType::RELAY, channel);
        char name[DEVICE_NAME_MAX_LENGTH] = {};
        snprintf(name, sizeof(name), "Relay %u", static_cast<unsigned>(channel));
        if (deviceRegistry.add(makeDevice(
                static_cast<DeviceId>(channel), 1U, name, binding)) != DeviceRegistryResult::SUCCESS ||
            resolver.registerOutput(binding, relaySlots[index].adapter) != DriverExecutionResult::SUCCESS)
            return false;
    }

    const DeviceBinding digitalBinding = makeBinding(DriverType::DIGITAL_INPUT, 1U);
    const DeviceBinding analogBinding = makeBinding(DriverType::ANALOG_INPUT, 1U);
    return deviceRegistry.add(makeDevice(101U, 2U, "Digital Input 1", digitalBinding)) ==
            DeviceRegistryResult::SUCCESS &&
        resolver.registerInput(digitalBinding, digitalInputAdapter) == DriverExecutionResult::SUCCESS &&
        deviceRegistry.add(makeDevice(201U, 3U, "Analog Input 1", analogBinding)) ==
            DeviceRegistryResult::SUCCESS &&
        resolver.registerInput(analogBinding, analogInputAdapter) == DriverExecutionResult::SUCCESS &&
        commandDispatcher.registerHandler(deviceRouteHandler) == CommandDispatchResult::SUCCESS;
}

void writeResult(const char* prefix, uint16_t code)
{
    Serial.print(prefix);
    Serial.println(code);
}

void executeSerialLine(uint32_t nowMs)
{
    if (serialOverflow)
    {
        Serial.println("ERR INPUT_TOO_LONG");
        return;
    }
    if (serialLength == 0U) return;

    RequestContext request;
    request.requestId = nextRequestId++;
    if (nextRequestId == INVALID_REQUEST_ID) nextRequestId = 1U;
    request.source = CommandSource::SERIAL;
    request.receivedTimestampMs = nowMs;
    request.setSourceName("serial");

    Command command;
    const CommandParseResult parseResult = commandParser.parse(
        serialLine, request, nextCommandId++, nowMs, command);
    if (nextCommandId == INVALID_COMMAND_ID) nextCommandId = 1U;
    if (parseResult != CommandParseResult::SUCCESS)
    {
        writeResult("ERR PARSE ", static_cast<uint16_t>(parseResult));
        return;
    }

    const CommandValidationResult validation = commandValidator.validate(command);
    if (validation != CommandValidationResult::VALID)
    {
        writeResult("ERR VALIDATE ", static_cast<uint16_t>(validation));
        return;
    }

    CommandResult result;
    const CommandDispatchResult dispatch = commandDispatcher.dispatch(command, result);
    if (dispatch != CommandDispatchResult::SUCCESS || !result.isSuccess())
    {
        writeResult("ERR EXECUTE ", static_cast<uint16_t>(dispatch));
        return;
    }

    Serial.print("OK ");
    Serial.println(serialLine);
}

void finishLine(uint32_t nowMs)
{
    serialLine[serialLength] = '\0';
    executeSerialLine(nowMs);
    memset(serialLine, 0, sizeof(serialLine));
    serialLength = 0U;
    serialOverflow = false;
}

void updateSerial(uint32_t nowMs)
{
    size_t processed = 0U;
    while (Serial.available() > 0 && processed < SERIAL_BYTES_PER_UPDATE)
    {
        const int incoming = Serial.read();
        ++processed;
        if (incoming < 0) break;
        const char value = static_cast<char>(incoming);
        if (value == '\r')
        {
            finishLine(nowMs);
            previousWasCr = true;
            continue;
        }
        if (value == '\n')
        {
            if (!previousWasCr) finishLine(nowMs);
            previousWasCr = false;
            continue;
        }
        previousWasCr = false;
        if (serialOverflow) continue;
        if (serialLength + 1U >= sizeof(serialLine))
        {
            serialOverflow = true;
            continue;
        }
        serialLine[serialLength++] = value;
    }
}

void printStartupReport()
{
    Serial.println();
    Serial.println("DELSAM Firmware v1 MVP");
    Serial.println("Command Runtime      READY");
    Serial.println("Serial Transport     READY");
    Serial.println("SMS Transport        NOT_CONFIGURED");
    Serial.println("SIM800               NOT_CONFIGURED");
    Serial.println("Relay 1-13,15-16     READY");
    Serial.println("Relay 14 / GPIO25    NOT_CONFIGURED");
    Serial.println("Digital Input 1      READY");
    Serial.println("Analog Input 1       READY");
    Serial.println("IR Receiver          READY");
    Serial.println("IR Sender            READY");
    Serial.println("RF / Sensors         NOT_CONFIGURED");
    Serial.println("SMS Security         NOT_CONFIGURED");
    Serial.println("System Mode          DEGRADED");
}
}

bool DelsamCompositionRoot::begin(uint32_t nowMs)
{
    (void)nowMs;
    if (ready) return true;
    for (size_t index = 0U; index < ACTIVE_RELAY_COUNT; ++index)
        relaySlots[index].driver.begin();
    digitalInput.begin();
    analogInput.begin();
    analogInput.setScale(
        BoardConfig::analogInputs[0].scaleMin,
        BoardConfig::analogInputs[0].scaleMax);
    irReceiver.begin();
    irSender.begin();
    ready = configureDevices();
    printStartupReport();
    if (!ready) Serial.println("Composition Root     FAILED");
    return ready;
}

void DelsamCompositionRoot::update(uint32_t nowMs)
{
    updateSerial(nowMs);
    if (!ready) return;
    for (size_t index = 0U; index < ACTIVE_RELAY_COUNT; ++index)
        relaySlots[index].driver.update();
    digitalInput.update();
    analogInput.update();
    irReceiver.update();
}

bool DelsamCompositionRoot::isReady()
{
    return ready;
}
