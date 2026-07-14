#include <DeviceCommandHandler.h>
#include <DeviceQueryHandler.h>
#include <AnalogInputAdapter.h>
#include <DigitalInputAdapter.h>
#include <RelayOutputAdapter.h>
#include <StaticDriverBindingResolver.h>
#include <Arduino.h>
#include <unity.h>

namespace
{
    DeviceTemplateRegistry templateRegistry;
    DeviceRegistry deviceRegistry(templateRegistry);
    StaticDriverBindingResolver resolver(1U);
    OutputManager outputManager(resolver);
    InputManager inputManager(resolver);
    DeviceCommandHandler commandHandler(
        deviceRegistry, templateRegistry, outputManager
    );
    DeviceQueryHandler queryHandler(
        deviceRegistry, templateRegistry, outputManager, inputManager
    );
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
        DeviceValueType valueType
    )
    {
        DeviceTemplate result{};
        result.id = id;
        result.setName("handler-template");
        result.valueType = valueType;
        addDriverType(result.allowedDriverTypes, driverType);
        addAction(result.allowedActions, DeviceAction::ON);
        addAction(result.allowedActions, DeviceAction::OFF);
        addAction(result.allowedActions, DeviceAction::TOGGLE);
        addAction(result.allowedActions, DeviceAction::PULSE);
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
        result.setName("handler-device");
        result.locationId = 1U;
        result.binding = binding;
        result.enabled = true;
        result.configured = true;
        result.health = DeviceHealth::OK;
        return result;
    }

    Command makeCommand(
        CommandDomain domain,
        uint16_t index,
        CommandOperation operation,
        CommandQueryType queryType = CommandQueryType::NONE
    )
    {
        Command result;
        result.context.commandId = 10U;
        result.context.request.requestId = 20U;
        result.context.request.source = CommandSource::SERIAL;
        result.context.createdTimestampMs = 30U;
        result.domain = domain;
        result.domainIndex = index;
        result.hasDomainIndex = index != 0U;
        result.operation = operation;
        result.queryType = queryType;
        return result;
    }

    void configureRegistries()
    {
        resolver.clear();
        deviceRegistry.clear();
        templateRegistry.clear();
        const DeviceBinding relayBinding = makeBinding(DriverType::RELAY, 1U);
        const DeviceBinding digitalBinding = makeBinding(DriverType::DIGITAL_INPUT, 2U);
        const DeviceBinding analogBinding = makeBinding(DriverType::ANALOG_INPUT, 3U);
        TEST_ASSERT_TRUE(templateRegistry.add(
            makeTemplate(1U, DriverType::RELAY, DeviceValueType::BOOLEAN)
        ));
        TEST_ASSERT_TRUE(templateRegistry.add(
            makeTemplate(2U, DriverType::DIGITAL_INPUT, DeviceValueType::BOOLEAN)
        ));
        TEST_ASSERT_TRUE(templateRegistry.add(
            makeTemplate(3U, DriverType::ANALOG_INPUT, DeviceValueType::INTEGER)
        ));
        TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(
            deviceRegistry.add(makeDevice(1U, 1U, relayBinding))
        ));
        TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(
            deviceRegistry.add(makeDevice(2U, 2U, digitalBinding))
        ));
        TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(
            deviceRegistry.add(makeDevice(3U, 3U, analogBinding))
        ));
        TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(
            resolver.registerOutput(relayBinding, relayAdapter)
        ));
        TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(
            resolver.registerInput(digitalBinding, digitalAdapter)
        ));
        TEST_ASSERT_EQUAL_UINT8(0U, static_cast<uint8_t>(
            resolver.registerInput(analogBinding, analogAdapter)
        ));
    }
}

void setUp() { configureRegistries(); }
void tearDown() {}

void test_command_validation_and_atomicity()
{
    CommandResult output;
    output.commandId = 99U;
    Command invalid;
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(DeviceCommandHandlerResult::INVALID_COMMAND),
        static_cast<uint8_t>(commandHandler.handle(invalid, output))
    );
    TEST_ASSERT_EQUAL_UINT32(99U, output.commandId);
    Command wrongDomain = makeCommand(CommandDomain::IN, 2U, CommandOperation::ON);
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(DeviceCommandHandlerResult::INVALID_DOMAIN),
        static_cast<uint8_t>(commandHandler.handle(wrongDomain, output))
    );
}

void test_action_duration_and_result_mapping()
{
    Command command = makeCommand(CommandDomain::OUT, 1U, CommandOperation::ON);
    CommandResult output;
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(commandHandler.handle(command, output)));
    TEST_ASSERT_TRUE(relay.isOn());
    TEST_ASSERT_TRUE(output.isSuccess());
    TEST_ASSERT_EQUAL_UINT32(10U, output.commandId);
    TEST_ASSERT_EQUAL_UINT32(20U, output.requestId);
    TEST_ASSERT_EQUAL_UINT32(30U, output.actualValue.timestampMs);

    command.setDuration(50U);
    output.commandId = 88U;
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(DeviceCommandHandlerResult::DURATION_NOT_SUPPORTED),
        static_cast<uint8_t>(commandHandler.handle(command, output))
    );
    TEST_ASSERT_EQUAL_UINT32(88U, output.commandId);
    command = makeCommand(CommandDomain::OUT, 1U, CommandOperation::PULSE);
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(DeviceCommandHandlerResult::DURATION_NOT_SUPPORTED),
        static_cast<uint8_t>(commandHandler.handle(command, output))
    );
    command.setDuration(25U);
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(commandHandler.handle(command, output)));
}

void test_query_routing_and_domain_consistency()
{
    CommandResult output;
    Command query = makeCommand(
        CommandDomain::OUT, 1U, CommandOperation::NONE, CommandQueryType::ITEM_INFO
    );
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(queryHandler.handle(query, output)));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DeviceValueType::BOOLEAN),
        static_cast<uint8_t>(output.actualValue.type));
    query = makeCommand(
        CommandDomain::IN, 2U, CommandOperation::NONE, CommandQueryType::ITEM_INFO
    );
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(queryHandler.handle(query, output)));
    query = makeCommand(
        CommandDomain::ADC, 3U, CommandOperation::NONE, CommandQueryType::ITEM_INFO
    );
    TEST_ASSERT_EQUAL_UINT8(0U,
        static_cast<uint8_t>(queryHandler.handle(query, output)));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DeviceValueType::INTEGER),
        static_cast<uint8_t>(output.actualValue.type));

    query = makeCommand(
        CommandDomain::IN, 1U, CommandOperation::NONE, CommandQueryType::ITEM_INFO
    );
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(DeviceCommandHandlerResult::DEVICE_TEMPLATE_MISMATCH),
        static_cast<uint8_t>(queryHandler.handle(query, output))
    );
}

void test_list_and_path_queries_are_unsupported()
{
    CommandResult output;
    Command list = makeCommand(
        CommandDomain::OUT, 0U, CommandOperation::NONE, CommandQueryType::LIST_ITEMS
    );
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(DeviceCommandHandlerResult::UNSUPPORTED_COMMAND),
        static_cast<uint8_t>(queryHandler.handle(list, output))
    );
    Command path = makeCommand(
        CommandDomain::OUT, 1U, CommandOperation::NONE, CommandQueryType::ITEM_INFO
    );
    TEST_ASSERT_TRUE(path.path.addSegment("state"));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(DeviceCommandHandlerResult::UNSUPPORTED_COMMAND),
        static_cast<uint8_t>(queryHandler.handle(path, output))
    );
}

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_command_validation_and_atomicity);
    RUN_TEST(test_action_duration_and_result_mapping);
    RUN_TEST(test_query_routing_and_domain_consistency);
    RUN_TEST(test_list_and_path_queries_are_unsupported);
    UNITY_END();
}

void loop() {}
