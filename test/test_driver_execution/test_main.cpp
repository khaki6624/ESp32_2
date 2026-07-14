#include <Arduino.h>
#include <unity.h>

#include <InputManager.h>
#include <OutputManager.h>
#include <StaticDriverBindingResolver.h>

class FakeOutputDriverPort final : public OutputDriverPort
{
public:
    DriverType driverType = DriverType::RELAY;
    DeviceValueType valueType = DeviceValueType::BOOLEAN;
    DriverPortHealth health = DriverPortHealth::READY;
    DeviceActionMask supportedActions = 0U;
    DriverExecutionResult executeResult = DriverExecutionResult::SUCCESS;
    DriverExecutionResult readResult = DriverExecutionResult::SUCCESS;
    DeviceValue actualValue = DeviceValue::makeBoolean(true, 10U);
    bool returnInvalidResponse = false;
    uint16_t executeCount = 0U;
    mutable uint16_t readCount = 0U;

    DriverType getDriverType() const override { return driverType; }
    bool supportsAction(DeviceAction action) const override
    {
        return hasAction(supportedActions, action);
    }
    DeviceValueType getValueType() const override { return valueType; }
    DriverPortHealth getHealth() const override { return health; }
    DriverExecutionResult execute(
        const DriverActionRequest&,
        DriverExecutionResponse& response
    ) override
    {
        ++executeCount;
        if (executeResult != DriverExecutionResult::SUCCESS)
            return executeResult;
        response.clear();
        response.result = DriverExecutionResult::SUCCESS;
        response.portHealth = DriverPortHealth::READY;
        response.completedTimestampMs = 20U;
        if (returnInvalidResponse)
        {
            response.hasActualValue = true;
            return DriverExecutionResult::SUCCESS;
        }
        response.actualValue = actualValue;
        response.hasActualValue = true;
        return DriverExecutionResult::SUCCESS;
    }
    DriverExecutionResult readActualValue(DeviceValue& output) const override
    {
        ++readCount;
        if (readResult != DriverExecutionResult::SUCCESS)
            return readResult;
        output = actualValue;
        return DriverExecutionResult::SUCCESS;
    }
};

class FakeInputDriverPort final : public InputDriverPort
{
public:
    DriverType driverType = DriverType::DIGITAL_INPUT;
    DeviceValueType valueType = DeviceValueType::BOOLEAN;
    DriverPortHealth health = DriverPortHealth::READY;
    DriverExecutionResult readResult = DriverExecutionResult::SUCCESS;
    DeviceValue value = DeviceValue::makeBoolean(true, 30U);
    mutable uint16_t readCount = 0U;

    DriverType getDriverType() const override { return driverType; }
    DeviceValueType getValueType() const override { return valueType; }
    DriverPortHealth getHealth() const override { return health; }
    DriverExecutionResult read(DeviceValue& output) const override
    {
        ++readCount;
        if (readResult != DriverExecutionResult::SUCCESS)
            return readResult;
        output = value;
        return DriverExecutionResult::SUCCESS;
    }
};

namespace
{
    DeviceBinding binding(NodeId node, DriverType type, uint8_t instance, uint8_t channel)
    {
        DeviceBinding result{};
        result.nodeId = node;
        result.driverType = type;
        result.driverInstance = instance;
        result.channel = channel;
        result.valid = true;
        return result;
    }

    DeviceTemplate deviceTemplate(DeviceTemplateId id, DriverType driver, DeviceValueType type)
    {
        DeviceTemplate result{};
        result.id = id;
        result.setName("test");
        result.valueType = type;
        addDriverType(result.allowedDriverTypes, driver);
        addAction(result.allowedActions, DeviceAction::ON);
        addAction(result.allowedActions, DeviceAction::PULSE);
        addAction(result.allowedActions, DeviceAction::SET_LEVEL);
        result.enabled = true;
        return result;
    }

    Device device(DeviceTemplateId templateId, const DeviceBinding& deviceBinding)
    {
        Device result{};
        result.id = 1U;
        result.templateId = templateId;
        result.setName("device");
        result.binding = deviceBinding;
        result.enabled = true;
        result.configured = true;
        result.health = DeviceHealth::OK;
        return result;
    }
}

void setUp() {}
void tearDown() {}

void test_request_and_response_contracts()
{
    DriverActionRequest request;
    TEST_ASSERT_FALSE(request.isValid());
    request.action = DeviceAction::ON;
    TEST_ASSERT_TRUE(request.isValid());
    request.hasValue = true;
    TEST_ASSERT_FALSE(request.isValid());
    request.clear();
    request.action = DeviceAction::ON;
    request.durationMs = 1U;
    TEST_ASSERT_FALSE(request.isValid());

    DriverExecutionResponse response;
    response.result = DriverExecutionResult::SUCCESS;
    response.portHealth = DriverPortHealth::READY;
    TEST_ASSERT_TRUE(response.isValid());
    response.hasActualValue = true;
    TEST_ASSERT_FALSE(response.isValid());
}

void test_driver_device_value_semantics()
{
    DeviceValue value;
    TEST_ASSERT_TRUE(isValidDriverDeviceValue(value));
    value.type = DeviceValueType::BOOLEAN;
    TEST_ASSERT_FALSE(isValidDriverDeviceValue(value));
    value.valid = true;
    value.type = DeviceValueType::NONE;
    TEST_ASSERT_FALSE(isValidDriverDeviceValue(value));
    TEST_ASSERT_TRUE(isValidDriverDeviceValue(DeviceValue::makeBoolean(true)));
    TEST_ASSERT_TRUE(isValidDriverDeviceValue(DeviceValue::makeInteger(-1)));
    TEST_ASSERT_TRUE(isValidDriverDeviceValue(DeviceValue::makeFloat(1.25f)));

    value = DeviceValue::makeFloat(1.0f);
    value.floatValue = NAN;
    TEST_ASSERT_FALSE(isValidDriverDeviceValue(value));
    value.floatValue = INFINITY;
    TEST_ASSERT_FALSE(isValidDriverDeviceValue(value));
    value.floatValue = -INFINITY;
    TEST_ASSERT_FALSE(isValidDriverDeviceValue(value));
    TEST_ASSERT_TRUE(isValidDriverDeviceValue(DeviceValue::makePercentage(0)));
    TEST_ASSERT_TRUE(isValidDriverDeviceValue(DeviceValue::makePercentage(100)));
    value = DeviceValue::makePercentage(100);
    value.percentageValue = 101U;
    TEST_ASSERT_FALSE(isValidDriverDeviceValue(value));
    TEST_ASSERT_TRUE(isValidDriverDeviceValue(DeviceValue::makeEnum(7)));
}

void test_request_response_reject_malformed_values()
{
    DriverActionRequest request;
    request.action = DeviceAction::SET_LEVEL;
    request.hasValue = true;
    request.value = DeviceValue::makeFloat(1.0f);
    request.value.floatValue = NAN;
    TEST_ASSERT_FALSE(request.isValid());
    request.value = DeviceValue::makePercentage(100);
    request.value.percentageValue = 200U;
    TEST_ASSERT_FALSE(request.isValid());
    request.hasValue = false;
    TEST_ASSERT_FALSE(request.isValid());
    request.value.clear();
    request.hasValue = true;
    request.value = DeviceValue::makePercentage(50);
    TEST_ASSERT_TRUE(request.isValid());

    DriverExecutionResponse response;
    response.result = DriverExecutionResult::SUCCESS;
    response.portHealth = DriverPortHealth::READY;
    response.hasActualValue = true;
    response.actualValue = DeviceValue::makeFloat(1.0f);
    response.actualValue.floatValue = NAN;
    TEST_ASSERT_FALSE(response.isValid());
    response.actualValue = DeviceValue::makePercentage(100);
    response.actualValue.percentageValue = 200U;
    TEST_ASSERT_FALSE(response.isValid());
    response.hasActualValue = false;
    TEST_ASSERT_FALSE(response.isValid());
}

void test_resolver_registration_resolution_and_removal()
{
    StaticDriverBindingResolver resolver(1U);
    FakeOutputDriverPort outputPort;
    FakeOutputDriverPort secondOutputPort;
    FakeInputDriverPort inputPort;
    const DeviceBinding outputBinding = binding(1U, DriverType::RELAY, 0U, 1U);
    const DeviceBinding secondBinding = binding(1U, DriverType::RELAY, 0U, 2U);
    const DeviceBinding inputBinding = binding(1U, DriverType::DIGITAL_INPUT, 0U, 1U);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::SUCCESS),
        static_cast<uint8_t>(resolver.registerOutput(outputBinding, outputPort)));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::SUCCESS),
        static_cast<uint8_t>(resolver.registerInput(inputBinding, inputPort)));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::BINDING_ALREADY_REGISTERED),
        static_cast<uint8_t>(resolver.registerOutput(outputBinding, secondOutputPort)));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::PORT_ALREADY_REGISTERED),
        static_cast<uint8_t>(resolver.registerOutput(secondBinding, outputPort)));
    TEST_ASSERT_EQUAL_UINT32(1U, resolver.outputCount());

    OutputDriverPort* resolvedOutput = nullptr;
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::SUCCESS),
        static_cast<uint8_t>(resolver.resolveOutput(outputBinding, resolvedOutput)));
    TEST_ASSERT_EQUAL_PTR(&outputPort, resolvedOutput);
    OutputDriverPort* sentinel = &secondOutputPort;
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::REMOTE_BINDING_NOT_SUPPORTED),
        static_cast<uint8_t>(resolver.resolveOutput(binding(2U, DriverType::RELAY, 0U, 1U), sentinel)));
    TEST_ASSERT_EQUAL_PTR(&secondOutputPort, sentinel);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::SUCCESS),
        static_cast<uint8_t>(resolver.unregisterOutput(outputBinding)));
    TEST_ASSERT_EQUAL_UINT32(0U, resolver.outputCount());
}

void test_output_manager_validation_health_duration_and_atomicity()
{
    StaticDriverBindingResolver resolver(1U);
    FakeOutputDriverPort port;
    addAction(port.supportedActions, DeviceAction::ON);
    addAction(port.supportedActions, DeviceAction::PULSE);
    const DeviceBinding b = binding(1U, DriverType::RELAY, 0U, 1U);
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(resolver.registerOutput(b, port)));
    Device d = device(1U, b);
    DeviceTemplate t = deviceTemplate(1U, DriverType::RELAY, DeviceValueType::BOOLEAN);
    OutputManager manager(resolver);
    DriverActionRequest request;
    request.action = DeviceAction::ON;
    DriverExecutionResponse response;
    response.completedTimestampMs = 99U;

    d.enabled = false;
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::DEVICE_DISABLED),
        static_cast<uint8_t>(manager.execute(d, t, request, response)));
    TEST_ASSERT_EQUAL_UINT16(0U, port.executeCount);
    d.enabled = true;
    port.health = DriverPortHealth::BUSY;
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::DRIVER_BUSY),
        static_cast<uint8_t>(manager.execute(d, t, request, response)));
    port.health = DriverPortHealth::READY;
    request.action = DeviceAction::PULSE;
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::INVALID_DURATION),
        static_cast<uint8_t>(manager.execute(d, t, request, response)));
    request.hasDuration = true;
    request.durationMs = 10U;
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(manager.execute(d, t, request, response)));
    TEST_ASSERT_EQUAL_UINT32(20U, response.completedTimestampMs);

    port.returnInvalidResponse = true;
    response.completedTimestampMs = 77U;
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::OUTPUT_VALUE_INVALID),
        static_cast<uint8_t>(manager.execute(d, t, request, response)));
    TEST_ASSERT_EQUAL_UINT32(77U, response.completedTimestampMs);
}

void test_input_manager_read_and_atomic_failure()
{
    StaticDriverBindingResolver resolver(1U);
    FakeInputDriverPort port;
    const DeviceBinding b = binding(1U, DriverType::DIGITAL_INPUT, 0U, 1U);
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(resolver.registerInput(b, port)));
    Device d = device(2U, b);
    DeviceTemplate t = deviceTemplate(2U, DriverType::DIGITAL_INPUT, DeviceValueType::BOOLEAN);
    InputManager manager(resolver);
    DeviceValue output = DeviceValue::makeBoolean(false, 99U);
    TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(manager.read(d, t, output)));
    TEST_ASSERT_EQUAL_UINT32(30U, output.timestampMs);

    port.readResult = DriverExecutionResult::DRIVER_ERROR;
    output = DeviceValue::makeBoolean(false, 88U);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::DRIVER_ERROR),
        static_cast<uint8_t>(manager.read(d, t, output)));
    TEST_ASSERT_EQUAL_UINT32(88U, output.timestampMs);
}

void test_managers_reject_malformed_driver_values()
{
    StaticDriverBindingResolver resolver(1U);
    FakeOutputDriverPort outputPort;
    outputPort.driverType = DriverType::ANALOG_OUTPUT;
    outputPort.valueType = DeviceValueType::FLOAT;
    addAction(outputPort.supportedActions, DeviceAction::ON);
    const DeviceBinding outputBinding = binding(1U, DriverType::ANALOG_OUTPUT, 0U, 1U);
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(resolver.registerOutput(outputBinding, outputPort)));
    Device outputDevice = device(3U, outputBinding);
    DeviceTemplate outputTemplate = deviceTemplate(
        3U, DriverType::ANALOG_OUTPUT, DeviceValueType::FLOAT
    );
    OutputManager outputManager(resolver);
    DriverActionRequest request;
    request.action = DeviceAction::ON;
    DriverExecutionResponse response;
    response.completedTimestampMs = 71U;
    outputPort.actualValue = DeviceValue::makeFloat(1.0f);
    outputPort.actualValue.floatValue = NAN;
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::OUTPUT_VALUE_INVALID),
        static_cast<uint8_t>(outputManager.execute(
            outputDevice, outputTemplate, request, response
        )));
    TEST_ASSERT_EQUAL_UINT32(71U, response.completedTimestampMs);
    outputPort.valueType = DeviceValueType::PERCENTAGE;
    outputTemplate.valueType = DeviceValueType::PERCENTAGE;
    outputPort.actualValue = DeviceValue::makePercentage(100);
    outputPort.actualValue.percentageValue = 200U;
    DeviceValue readOutput = DeviceValue::makePercentage(25, 73U);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::OUTPUT_VALUE_INVALID),
        static_cast<uint8_t>(outputManager.readActualValue(
            outputDevice, outputTemplate, readOutput
        )));
    TEST_ASSERT_EQUAL_UINT32(73U, readOutput.timestampMs);

    FakeInputDriverPort inputPort;
    inputPort.driverType = DriverType::ANALOG_INPUT;
    inputPort.valueType = DeviceValueType::FLOAT;
    const DeviceBinding inputBinding = binding(1U, DriverType::ANALOG_INPUT, 0U, 1U);
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(resolver.registerInput(inputBinding, inputPort)));
    Device inputDevice = device(4U, inputBinding);
    DeviceTemplate inputTemplate = deviceTemplate(
        4U, DriverType::ANALOG_INPUT, DeviceValueType::FLOAT
    );
    InputManager inputManager(resolver);
    DeviceValue output = DeviceValue::makeFloat(2.0f, 72U);
    inputPort.value = DeviceValue::makeFloat(1.0f);
    inputPort.value.floatValue = INFINITY;
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::INPUT_VALUE_INVALID),
        static_cast<uint8_t>(inputManager.read(inputDevice, inputTemplate, output)));
    TEST_ASSERT_EQUAL_UINT32(72U, output.timestampMs);
    inputPort.valueType = DeviceValueType::PERCENTAGE;
    inputTemplate.valueType = DeviceValueType::PERCENTAGE;
    inputPort.value = DeviceValue::makePercentage(100);
    inputPort.value.percentageValue = 200U;
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::INPUT_VALUE_INVALID),
        static_cast<uint8_t>(inputManager.read(inputDevice, inputTemplate, output)));
    TEST_ASSERT_EQUAL_UINT32(72U, output.timestampMs);
    inputPort.valueType = DeviceValueType::FLOAT;
    inputTemplate.valueType = DeviceValueType::FLOAT;
    inputPort.value = DeviceValue::makeInteger(5);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverExecutionResult::VALUE_TYPE_MISMATCH),
        static_cast<uint8_t>(inputManager.read(inputDevice, inputTemplate, output)));
    TEST_ASSERT_EQUAL_UINT32(72U, output.timestampMs);
}

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_request_and_response_contracts);
    RUN_TEST(test_driver_device_value_semantics);
    RUN_TEST(test_request_response_reject_malformed_values);
    RUN_TEST(test_resolver_registration_resolution_and_removal);
    RUN_TEST(test_output_manager_validation_health_duration_and_atomicity);
    RUN_TEST(test_input_manager_read_and_atomic_failure);
    RUN_TEST(test_managers_reject_malformed_driver_values);
    UNITY_END();
}

void loop() {}
