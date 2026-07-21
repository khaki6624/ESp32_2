#include <Arduino.h>
#ifdef INPUT
#undef INPUT
#endif
#include <unity.h>

#include <AnalogInputAdapter.h>
#include <DigitalInputAdapter.h>
#include <InputManager.h>
#include <OutputManager.h>
#include <RelayOutputAdapter.h>
#include <StaticDriverBindingResolver.h>

namespace
{
    Relay relay(16U);
    DigitalInput digitalInput(17U);
    AnalogInput analogInput(34U);
    RelayOutputAdapter relayAdapter(relay);
    DigitalInputAdapter digitalAdapter(digitalInput);
    AnalogInputAdapter analogAdapter(analogInput);

    DeviceBinding makeBinding(DriverType type, uint8_t channel)
    {
        DeviceBinding result{};
        result.nodeId = 1U;
        result.driverType = type;
        result.channel = channel;
        result.valid = true;
        return result;
    }

    DeviceTemplate makeTemplate(
        DeviceTemplateId id,
        DriverType driverType,
        DeviceValueType valueType,
        DeviceAction action = DeviceAction::READ
    )
    {
        DeviceTemplate result{};
        result.id = id;
        result.setName("adapter-test");
        result.valueType = valueType;
        addDriverType(result.allowedDriverTypes, driverType);
        addAction(result.allowedActions, action);
        result.enabled = true;
        return result;
    }

    Device makeDevice(
        DeviceId id,
        DeviceTemplateId templateId,
        const DeviceBinding& binding
    )
    {
        Device result{};
        result.id = id;
        result.templateId = templateId;
        result.setName("adapter-device");
        result.binding = binding;
        result.enabled = true;
        result.configured = true;
        result.health = DeviceHealth::OK;
        return result;
    }
}

void setUp() {}
void tearDown() {}

void test_relay_metadata_and_action_validation()
{
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverType::RELAY),
        static_cast<uint8_t>(relayAdapter.getDriverType()));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DeviceValueType::BOOLEAN),
        static_cast<uint8_t>(relayAdapter.getValueType()));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverPortHealth::READY),
        static_cast<uint8_t>(relayAdapter.getHealth()));
    TEST_ASSERT_TRUE(relayAdapter.supportsAction(DeviceAction::ON));
    TEST_ASSERT_TRUE(relayAdapter.supportsAction(DeviceAction::OFF));
    TEST_ASSERT_TRUE(relayAdapter.supportsAction(DeviceAction::TOGGLE));
    TEST_ASSERT_TRUE(relayAdapter.supportsAction(DeviceAction::PULSE));
    TEST_ASSERT_FALSE(relayAdapter.supportsAction(DeviceAction::SET_LEVEL));
    TEST_ASSERT_FALSE(relayAdapter.supportsAction(DeviceAction::READ));

    DriverActionRequest request;
    request.action = DeviceAction::PULSE;
    DriverExecutionResponse response;
    response.completedTimestampMs = 99U;
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::INVALID_DURATION),
        static_cast<uint8_t>(relayAdapter.execute(request, response)));
    TEST_ASSERT_EQUAL_UINT32(99U, response.completedTimestampMs);
    request.hasDuration = true;
    request.durationMs = 0U;
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::INVALID_DURATION),
        static_cast<uint8_t>(relayAdapter.execute(request, response)));
}

void test_relay_execute_and_read()
{
    DriverActionRequest request;
    DriverExecutionResponse response;
    request.action = DeviceAction::ON;
    request.requestedTimestampMs = 123U;
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(relayAdapter.execute(request, response)));
    TEST_ASSERT_TRUE(relay.isOn());
    TEST_ASSERT_TRUE(response.actualValue.booleanValue);
    TEST_ASSERT_EQUAL_UINT32(123U, response.completedTimestampMs);
    TEST_ASSERT_EQUAL_UINT32(123U, response.actualValue.timestampMs);

    request.action = DeviceAction::TOGGLE;
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(relayAdapter.execute(request, response)));
    TEST_ASSERT_FALSE(relay.isOn());
    request.action = DeviceAction::PULSE;
    request.hasDuration = true;
    request.durationMs = 50U;
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(relayAdapter.execute(request, response)));
    TEST_ASSERT_TRUE(relay.isOn());

    DeviceValue output;
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(relayAdapter.readActualValue(output)));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DeviceValueType::BOOLEAN),
        static_cast<uint8_t>(output.type));
    TEST_ASSERT_EQUAL_UINT32(0U, output.timestampMs);
}

void test_input_metadata_and_reads()
{
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverType::DIGITAL_INPUT),
        static_cast<uint8_t>(digitalAdapter.getDriverType()));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DeviceValueType::BOOLEAN),
        static_cast<uint8_t>(digitalAdapter.getValueType()));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverType::ANALOG_INPUT),
        static_cast<uint8_t>(analogAdapter.getDriverType()));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DeviceValueType::INTEGER),
        static_cast<uint8_t>(analogAdapter.getValueType()));

    DeviceValue output;
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(digitalAdapter.read(output)));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DeviceValueType::BOOLEAN),
        static_cast<uint8_t>(output.type));
    TEST_ASSERT_EQUAL_UINT32(0U, output.timestampMs);
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(analogAdapter.read(output)));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DeviceValueType::INTEGER),
        static_cast<uint8_t>(output.type));
    TEST_ASSERT_EQUAL_UINT32(0U, output.timestampMs);
}

void test_resolver_and_manager_integration()
{
    StaticDriverBindingResolver resolver(1U);
    const DeviceBinding relayBinding = makeBinding(DriverType::RELAY, 1U);
    const DeviceBinding digitalBinding = makeBinding(DriverType::DIGITAL_INPUT, 2U);
    const DeviceBinding analogBinding = makeBinding(DriverType::ANALOG_INPUT, 3U);
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(resolver.registerOutput(relayBinding, relayAdapter)));
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(resolver.registerInput(digitalBinding, digitalAdapter)));
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(resolver.registerInput(analogBinding, analogAdapter)));

    OutputDriverPort* resolvedOutput = nullptr;
    InputDriverPort* resolvedInput = nullptr;
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(resolver.resolveOutput(relayBinding, resolvedOutput)));
    TEST_ASSERT_EQUAL_PTR(&relayAdapter, resolvedOutput);
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(resolver.resolveInput(digitalBinding, resolvedInput)));
    TEST_ASSERT_EQUAL_PTR(&digitalAdapter, resolvedInput);
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(resolver.resolveInput(analogBinding, resolvedInput)));
    TEST_ASSERT_EQUAL_PTR(&analogAdapter, resolvedInput);

    OutputManager outputManager(resolver);
    InputManager inputManager(resolver);
    Device relayDevice = makeDevice(1U, 1U, relayBinding);
    DeviceTemplate relayTemplate = makeTemplate(
        1U, DriverType::RELAY, DeviceValueType::BOOLEAN, DeviceAction::ON
    );
    DriverActionRequest request;
    request.action = DeviceAction::ON;
    DriverExecutionResponse response;
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(outputManager.execute(
        relayDevice, relayTemplate, request, response
    )));
    DeviceValue output;
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(outputManager.readActualValue(
        relayDevice, relayTemplate, output
    )));

    Device digitalDevice = makeDevice(2U, 2U, digitalBinding);
    DeviceTemplate digitalTemplate = makeTemplate(
        2U, DriverType::DIGITAL_INPUT, DeviceValueType::BOOLEAN
    );
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(inputManager.read(
        digitalDevice, digitalTemplate, output
    )));
    Device analogDevice = makeDevice(3U, 3U, analogBinding);
    DeviceTemplate analogTemplate = makeTemplate(
        3U, DriverType::ANALOG_INPUT, DeviceValueType::INTEGER
    );
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(inputManager.read(
        analogDevice, analogTemplate, output
    )));
}

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_relay_metadata_and_action_validation);
    RUN_TEST(test_relay_execute_and_read);
    RUN_TEST(test_input_metadata_and_reads);
    RUN_TEST(test_resolver_and_manager_integration);
    UNITY_END();
}

void loop() {}
