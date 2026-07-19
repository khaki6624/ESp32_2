#include <Arduino.h>
#include <ConfigRuntime.h>
#include <limits.h>
#include <string.h>
#include <type_traits>
#include <unity.h>

namespace
{
enum : ConfigId
{
    ENABLED_ID = 1U, PULSE_ID, OFFSET_ID, NAME_ID, MODEL_ID, ADDRESS_ID
};

void assertResult(ConfigResult expected, ConfigResult actual)
{ TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected), static_cast<uint8_t>(actual)); }

ConfigKey key(const char* text)
{
    ConfigKey result;
    TEST_ASSERT_TRUE(result.set(text));
    return result;
}

void registerIntegrationDefinitions(ConfigRegistry& registry)
{
    assertResult(ConfigResult::SUCCESS, registry.registerDefinition(
        ConfigDefinition::booleanConfig(ENABLED_ID, "CFG.OUT1.ENABLED", true,
            ConfigAccess::READ_WRITE)));
    assertResult(ConfigResult::SUCCESS, registry.registerDefinition(
        ConfigDefinition::uint32Config(PULSE_ID, "CFG.OUT1.PULSE_MS", 1000U,
            100U, 60000U, ConfigAccess::READ_WRITE)));
    assertResult(ConfigResult::SUCCESS, registry.registerDefinition(
        ConfigDefinition::int32Config(OFFSET_ID, "CFG.TEMP.OFFSET", 0,
            -20, 20, ConfigAccess::READ_WRITE)));
    assertResult(ConfigResult::SUCCESS, registry.registerDefinition(
        ConfigDefinition::stringConfig(NAME_ID, "CFG.OUT1.NAME", "OUT1",
            1U, 32U, false, ConfigAccess::READ_WRITE)));
    assertResult(ConfigResult::SUCCESS, registry.registerDefinition(
        ConfigDefinition::stringConfig(MODEL_ID, "CFG.SYSTEM.MODEL", "DELSAM",
            1U, 32U, false, ConfigAccess::READ_ONLY)));
    assertResult(ConfigResult::SUCCESS, registry.registerDefinition(
        ConfigDefinition::uint32Config(ADDRESS_ID, "CFG.NODE1.ADDRESS", 1U,
            1U, 247U, ConfigAccess::WRITE_ONCE)));
}

uint32_t readUInt(const ConfigRuntime& runtime, ConfigId id)
{
    ConfigValue value;
    uint32_t result = 0U;
    assertResult(ConfigResult::SUCCESS, runtime.getById(id, value));
    TEST_ASSERT_TRUE(value.getUInt32(result));
    return result;
}
}

void test_config_key_validation_boundaries_and_atomic_set()
{
    ConfigKey value;
    TEST_ASSERT_FALSE(value.isValid());
    TEST_ASSERT_FALSE(value.set(nullptr));
    TEST_ASSERT_FALSE(value.set(""));
    TEST_ASSERT_TRUE(value.set("CFG.OUT1_NAME.9"));
    TEST_ASSERT_TRUE(value.isValid());
    TEST_ASSERT_EQUAL_STRING("CFG.OUT1_NAME.9", value.c_str());
    TEST_ASSERT_EQUAL_UINT32(strlen("CFG.OUT1_NAME.9"), value.length());

    const char* invalid[] = {"cfg.OUT", "CFG OUT", "CFG/OUT", "CFG\\OUT",
        "CFG-OUT", "CFG=OUT", "CFG|OUT", "CFG\rOUT", "CFG\nOUT"};
    for (size_t index = 0U; index < sizeof(invalid) / sizeof(invalid[0]); ++index)
    {
        TEST_ASSERT_FALSE(value.set(invalid[index]));
        TEST_ASSERT_EQUAL_STRING("CFG.OUT1_NAME.9", value.c_str());
    }
    char maximum[CONFIG_KEY_MAX_LENGTH] = {};
    memset(maximum, 'A', sizeof(maximum) - 1U);
    TEST_ASSERT_TRUE(value.set(maximum));
    TEST_ASSERT_EQUAL_UINT32(CONFIG_KEY_MAX_LENGTH - 1U, value.length());
    char unterminated[CONFIG_KEY_MAX_LENGTH];
    memset(unterminated, 'A', sizeof(unterminated));
    TEST_ASSERT_FALSE(value.set(unterminated));
    TEST_ASSERT_EQUAL_UINT32(CONFIG_KEY_MAX_LENGTH - 1U, value.length());
    ConfigKey equal; TEST_ASSERT_TRUE(equal.set(maximum));
    ConfigKey different; TEST_ASSERT_TRUE(different.set("CFG.OTHER"));
    TEST_ASSERT_TRUE(value.equals(equal));
    TEST_ASSERT_FALSE(value.equals(different));
}

void test_config_value_is_type_safe_bounded_and_binary_preserving()
{
    ConfigValue invalid;
    TEST_ASSERT_FALSE(invalid.isValid());
    bool boolean = true;
    uint32_t unsignedValue = 77U;
    int32_t signedValue = 88;
    const char* stringValue = "unchanged";
    TEST_ASSERT_FALSE(invalid.getBool(boolean));
    TEST_ASSERT_TRUE(boolean);

    const ConfigValue boolValue = ConfigValue::fromBool(false);
    TEST_ASSERT_TRUE(boolValue.getBool(boolean)); TEST_ASSERT_FALSE(boolean);
    TEST_ASSERT_FALSE(boolValue.getUInt32(unsignedValue)); TEST_ASSERT_EQUAL_UINT32(77U, unsignedValue);
    const ConfigValue maximumUnsigned = ConfigValue::fromUInt32(UINT32_MAX);
    TEST_ASSERT_TRUE(maximumUnsigned.getUInt32(unsignedValue)); TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, unsignedValue);
    const ConfigValue minimumSigned = ConfigValue::fromInt32(INT32_MIN);
    TEST_ASSERT_TRUE(minimumSigned.getInt32(signedValue)); TEST_ASSERT_EQUAL_INT32(INT32_MIN, signedValue);
    const ConfigValue maximumSigned = ConfigValue::fromInt32(INT32_MAX);
    TEST_ASSERT_TRUE(maximumSigned.getInt32(signedValue)); TEST_ASSERT_EQUAL_INT32(INT32_MAX, signedValue);
    TEST_ASSERT_FALSE(maximumSigned.getString(stringValue)); TEST_ASSERT_EQUAL_STRING("unchanged", stringValue);

    const char utf8[] = "\xD8\xAF\xD9\x84\xD8\xB3\xD8\xA7\xD9\x85";
    const ConfigValue text = ConfigValue::fromString(utf8);
    TEST_ASSERT_TRUE(text.getString(stringValue)); TEST_ASSERT_EQUAL_MEMORY(utf8, stringValue, sizeof(utf8));
    TEST_ASSERT_TRUE(text.equals(ConfigValue::fromString(utf8)));
    TEST_ASSERT_FALSE(text.equals(boolValue));
    TEST_ASSERT_TRUE(ConfigValue::fromString("").isValid());
    TEST_ASSERT_FALSE(ConfigValue::fromString(nullptr).isValid());
    char maximum[CONFIG_STRING_MAX_LENGTH] = {};
    memset(maximum, 'X', sizeof(maximum) - 1U);
    TEST_ASSERT_TRUE(ConfigValue::fromString(maximum).isValid());
    char tooLong[CONFIG_STRING_MAX_LENGTH]; memset(tooLong, 'X', sizeof(tooLong));
    TEST_ASSERT_FALSE(ConfigValue::fromString(tooLong).isValid());
}

void test_definition_factories_reject_invalid_defaults_and_constraints()
{
    TEST_ASSERT_FALSE(ConfigDefinition{}.isValid());
    TEST_ASSERT_TRUE(ConfigDefinition::booleanConfig(1U, "CFG.BOOL", true,
        ConfigAccess::READ_WRITE).isValid());
    TEST_ASSERT_TRUE(ConfigDefinition::uint32Config(2U, "CFG.UINT", 5U, 1U, 9U,
        ConfigAccess::READ_WRITE).isValid());
    TEST_ASSERT_TRUE(ConfigDefinition::int32Config(3U, "CFG.INT", -1, -4, 4,
        ConfigAccess::WRITE_ONCE).isValid());
    TEST_ASSERT_TRUE(ConfigDefinition::stringConfig(4U, "CFG.TEXT", "OK", 1U, 8U,
        false, ConfigAccess::READ_ONLY).isValid());
    TEST_ASSERT_FALSE(ConfigDefinition::booleanConfig(0U, "CFG.BAD", true,
        ConfigAccess::READ_WRITE).isValid());
    TEST_ASSERT_FALSE(ConfigDefinition::booleanConfig(1U, "cfg.bad", true,
        ConfigAccess::READ_WRITE).isValid());
    TEST_ASSERT_FALSE(ConfigDefinition::booleanConfig(1U, "CFG.BAD", true,
        ConfigAccess::COUNT).isValid());
    TEST_ASSERT_FALSE(ConfigDefinition::uint32Config(1U, "CFG.BAD", 5U, 9U, 1U,
        ConfigAccess::READ_WRITE).isValid());
    TEST_ASSERT_FALSE(ConfigDefinition::uint32Config(1U, "CFG.BAD", 10U, 1U, 9U,
        ConfigAccess::READ_WRITE).isValid());
    TEST_ASSERT_FALSE(ConfigDefinition::int32Config(1U, "CFG.BAD", -10, -9, 9,
        ConfigAccess::READ_WRITE).isValid());
    TEST_ASSERT_FALSE(ConfigDefinition::stringConfig(1U, "CFG.BAD", "", 0U, 8U,
        false, ConfigAccess::READ_WRITE).isValid());
    TEST_ASSERT_FALSE(ConfigDefinition::stringConfig(1U, "CFG.BAD", "OK", 1U,
        CONFIG_STRING_MAX_LENGTH, false, ConfigAccess::READ_WRITE).isValid());
}

void test_validator_enforces_exact_type_and_inclusive_boundaries()
{
    ConfigValidator validator;
    const ConfigDefinition boolean = ConfigDefinition::booleanConfig(1U, "CFG.B",
        false, ConfigAccess::READ_WRITE);
    const ConfigDefinition unsignedDefinition = ConfigDefinition::uint32Config(2U,
        "CFG.U", 5U, 5U, 10U, ConfigAccess::READ_WRITE);
    const ConfigDefinition signedDefinition = ConfigDefinition::int32Config(3U,
        "CFG.I", 0, -2, 2, ConfigAccess::READ_WRITE);
    const ConfigDefinition stringDefinition = ConfigDefinition::stringConfig(4U,
        "CFG.S", "AB", 2U, 4U, false, ConfigAccess::READ_WRITE);
    const ConfigDefinition emptyAllowed = ConfigDefinition::stringConfig(5U,
        "CFG.EMPTY", "", 2U, 4U, true, ConfigAccess::READ_WRITE);
    assertResult(ConfigResult::SUCCESS, validator.validate(boolean, ConfigValue::fromBool(true)));
    assertResult(ConfigResult::TYPE_MISMATCH, validator.validate(boolean, ConfigValue::fromUInt32(1U)));
    assertResult(ConfigResult::INVALID_VALUE, validator.validate(boolean, ConfigValue{}));
    assertResult(ConfigResult::SUCCESS, validator.validate(unsignedDefinition, ConfigValue::fromUInt32(5U)));
    assertResult(ConfigResult::SUCCESS, validator.validate(unsignedDefinition, ConfigValue::fromUInt32(10U)));
    assertResult(ConfigResult::INVALID_VALUE, validator.validate(unsignedDefinition, ConfigValue::fromUInt32(4U)));
    assertResult(ConfigResult::INVALID_VALUE, validator.validate(unsignedDefinition, ConfigValue::fromUInt32(11U)));
    assertResult(ConfigResult::SUCCESS, validator.validate(signedDefinition, ConfigValue::fromInt32(-2)));
    assertResult(ConfigResult::SUCCESS, validator.validate(signedDefinition, ConfigValue::fromInt32(2)));
    assertResult(ConfigResult::INVALID_VALUE, validator.validate(signedDefinition, ConfigValue::fromInt32(3)));
    assertResult(ConfigResult::SUCCESS, validator.validate(stringDefinition, ConfigValue::fromString("AB")));
    assertResult(ConfigResult::SUCCESS, validator.validate(stringDefinition, ConfigValue::fromString("ABCD")));
    assertResult(ConfigResult::INVALID_VALUE, validator.validate(stringDefinition, ConfigValue::fromString("A")));
    assertResult(ConfigResult::INVALID_VALUE, validator.validate(stringDefinition, ConfigValue::fromString("ABCDE")));
    assertResult(ConfigResult::SUCCESS, validator.validate(emptyAllowed, ConfigValue::fromString("")));
    assertResult(ConfigResult::INVALID_VALUE, validator.validate(emptyAllowed, ConfigValue::fromString("A")));
}

void test_registry_is_ordered_atomic_and_bounded()
{
    ConfigRegistry registry;
    TEST_ASSERT_EQUAL_UINT32(0U, registry.size());
    TEST_ASSERT_EQUAL_UINT32(CONFIG_REGISTRY_MAX_ENTRIES, registry.capacity());
    TEST_ASSERT_FALSE(registry.isFull());
    const ConfigDefinition first = ConfigDefinition::booleanConfig(1U, "CFG.FIRST", true,
        ConfigAccess::READ_WRITE);
    assertResult(ConfigResult::SUCCESS, registry.registerDefinition(first));
    TEST_ASSERT_EQUAL_PTR(registry.getAt(0U), registry.findById(1U));
    TEST_ASSERT_EQUAL_PTR(registry.getAt(0U), registry.findByKey(key("CFG.FIRST")));
    TEST_ASSERT_NULL(registry.findById(0U));
    TEST_ASSERT_NULL(registry.findByKey(key("CFG.MISSING")));
    assertResult(ConfigResult::DUPLICATE_ID, registry.registerDefinition(
        ConfigDefinition::int32Config(1U, "CFG.SECOND", 0, -1, 1, ConfigAccess::READ_WRITE)));
    assertResult(ConfigResult::DUPLICATE_KEY, registry.registerDefinition(
        ConfigDefinition::booleanConfig(2U, "CFG.FIRST", false, ConfigAccess::READ_WRITE)));
    assertResult(ConfigResult::INVALID_VALUE, registry.registerDefinition(ConfigDefinition{}));
    TEST_ASSERT_EQUAL_UINT32(1U, registry.size());

    for (size_t index = 1U; index < CONFIG_REGISTRY_MAX_ENTRIES; ++index)
    {
        char generated[] = "CFG.CAP.00";
        generated[8] = static_cast<char>('0' + ((index / 10U) % 10U));
        generated[9] = static_cast<char>('0' + (index % 10U));
        assertResult(ConfigResult::SUCCESS, registry.registerDefinition(
            ConfigDefinition::uint32Config(static_cast<ConfigId>(index + 1U), generated,
                0U, 0U, UINT32_MAX, ConfigAccess::READ_WRITE)));
    }
    TEST_ASSERT_TRUE(registry.isFull());
    TEST_ASSERT_EQUAL_UINT16(CONFIG_REGISTRY_MAX_ENTRIES, registry.getAt(
        CONFIG_REGISTRY_MAX_ENTRIES - 1U)->id());
    assertResult(ConfigResult::REGISTRY_FULL, registry.registerDefinition(
        ConfigDefinition::booleanConfig(100U, "CFG.OVERFLOW", true, ConfigAccess::READ_WRITE)));
}

void test_runtime_begin_get_set_reset_and_access_policies()
{
    ConfigRegistry registry; registerIntegrationDefinitions(registry);
    ConfigRuntime runtime(registry);
    ConfigValue output = ConfigValue::fromString("sentinel");
    TEST_ASSERT_FALSE(runtime.isInitialized());
    assertResult(ConfigResult::NOT_INITIALIZED, runtime.getById(ENABLED_ID, output));
    assertResult(ConfigResult::NOT_INITIALIZED, runtime.setById(ENABLED_ID, ConfigValue::fromBool(false)));
    assertResult(ConfigResult::NOT_INITIALIZED, runtime.resetToDefault(ENABLED_ID));
    assertResult(ConfigResult::SUCCESS, runtime.begin()); TEST_ASSERT_TRUE(runtime.isInitialized());

    bool enabled = false; assertResult(ConfigResult::SUCCESS, runtime.getById(ENABLED_ID, output));
    TEST_ASSERT_TRUE(output.getBool(enabled)); TEST_ASSERT_TRUE(enabled);
    TEST_ASSERT_EQUAL_UINT32(1000U, readUInt(runtime, PULSE_ID));
    int32_t offset = 99; assertResult(ConfigResult::SUCCESS, runtime.getById(OFFSET_ID, output));
    TEST_ASSERT_TRUE(output.getInt32(offset)); TEST_ASSERT_EQUAL_INT32(0, offset);
    const char* text = nullptr; assertResult(ConfigResult::SUCCESS, runtime.getByKey(key("CFG.OUT1.NAME"), output));
    TEST_ASSERT_TRUE(output.getString(text)); TEST_ASSERT_EQUAL_STRING("OUT1", text);

    assertResult(ConfigResult::SUCCESS, runtime.setById(ENABLED_ID, ConfigValue::fromBool(false)));
    assertResult(ConfigResult::SUCCESS, runtime.setByKey(key("CFG.OUT1.PULSE_MS"), ConfigValue::fromUInt32(2500U)));
    assertResult(ConfigResult::SUCCESS, runtime.setById(OFFSET_ID, ConfigValue::fromInt32(-5)));
    assertResult(ConfigResult::SUCCESS, runtime.setById(NAME_ID, ConfigValue::fromString("PUMP")));
    assertResult(ConfigResult::READ_ONLY, runtime.setById(MODEL_ID, ConfigValue::fromString("OTHER")));
    assertResult(ConfigResult::TYPE_MISMATCH, runtime.setById(PULSE_ID, ConfigValue::fromInt32(500)));
    assertResult(ConfigResult::INVALID_VALUE, runtime.setById(PULSE_ID, ConfigValue::fromUInt32(99U)));
    TEST_ASSERT_EQUAL_UINT32(2500U, readUInt(runtime, PULSE_ID));
    assertResult(ConfigResult::INVALID_ID, runtime.getById(0U, output));
    assertResult(ConfigResult::NOT_FOUND, runtime.getById(99U, output));
    ConfigKey invalidKey; assertResult(ConfigResult::INVALID_KEY, runtime.getByKey(invalidKey, output));

    assertResult(ConfigResult::SUCCESS, runtime.resetToDefault(PULSE_ID));
    TEST_ASSERT_EQUAL_UINT32(1000U, readUInt(runtime, PULSE_ID));
    assertResult(ConfigResult::SUCCESS, runtime.resetToDefault(MODEL_ID));
    assertResult(ConfigResult::SUCCESS, runtime.resetAllToDefaults());
    assertResult(ConfigResult::SUCCESS, runtime.begin());
    TEST_ASSERT_EQUAL_UINT32(1000U, readUInt(runtime, PULSE_ID));
}

void test_write_once_locks_only_after_success_and_resets_cleanly()
{
    ConfigRegistry registry; registerIntegrationDefinitions(registry);
    ConfigRuntime runtime(registry); assertResult(ConfigResult::SUCCESS, runtime.begin());
    assertResult(ConfigResult::TYPE_MISMATCH, runtime.setById(ADDRESS_ID, ConfigValue::fromInt32(2)));
    assertResult(ConfigResult::INVALID_VALUE, runtime.setById(ADDRESS_ID, ConfigValue::fromUInt32(0U)));
    assertResult(ConfigResult::SUCCESS, runtime.setById(ADDRESS_ID, ConfigValue::fromUInt32(1U)));
    assertResult(ConfigResult::ALREADY_SET, runtime.setById(ADDRESS_ID, ConfigValue::fromUInt32(2U)));
    assertResult(ConfigResult::SUCCESS, runtime.resetToDefault(ADDRESS_ID));
    assertResult(ConfigResult::SUCCESS, runtime.setById(ADDRESS_ID, ConfigValue::fromUInt32(2U)));
    assertResult(ConfigResult::SUCCESS, runtime.resetAllToDefaults());
    assertResult(ConfigResult::SUCCESS, runtime.setById(ADDRESS_ID, ConfigValue::fromUInt32(3U)));
    assertResult(ConfigResult::SUCCESS, runtime.begin());
    assertResult(ConfigResult::SUCCESS, runtime.setById(ADDRESS_ID, ConfigValue::fromUInt32(4U)));
}

void test_registry_mutation_is_detected_before_read_or_commit_and_rebegin_resyncs()
{
    ConfigRegistry registry;
    registry.registerDefinition(ConfigDefinition::uint32Config(1U, "CFG.ONE", 1U,
        0U, 10U, ConfigAccess::READ_WRITE));
    ConfigRuntime runtime(registry); assertResult(ConfigResult::SUCCESS, runtime.begin());
    ConfigValue output = ConfigValue::fromString("sentinel");
    registry.registerDefinition(ConfigDefinition::booleanConfig(2U, "CFG.TWO", true,
        ConfigAccess::READ_WRITE));
    assertResult(ConfigResult::INTERNAL_ERROR, runtime.getById(1U, output));
    const char* unchanged = nullptr; TEST_ASSERT_TRUE(output.getString(unchanged));
    TEST_ASSERT_EQUAL_STRING("sentinel", unchanged);
    assertResult(ConfigResult::INTERNAL_ERROR, runtime.setById(1U, ConfigValue::fromUInt32(5U)));
    assertResult(ConfigResult::INTERNAL_ERROR, runtime.resetAllToDefaults());
    assertResult(ConfigResult::SUCCESS, runtime.begin());
    TEST_ASSERT_EQUAL_UINT32(1U, readUInt(runtime, 1U));
    bool second = false; assertResult(ConfigResult::SUCCESS, runtime.getById(2U, output));
    TEST_ASSERT_TRUE(output.getBool(second)); TEST_ASSERT_TRUE(second);
}

void test_empty_and_full_registry_runtime_boundaries()
{
    ConfigRegistry emptyRegistry; ConfigRuntime empty(emptyRegistry);
    assertResult(ConfigResult::SUCCESS, empty.begin());
    assertResult(ConfigResult::SUCCESS, empty.resetAllToDefaults());
    ConfigValue output; assertResult(ConfigResult::NOT_FOUND, empty.getById(1U, output));

    ConfigRegistry fullRegistry;
    for (size_t index = 0U; index < CONFIG_REGISTRY_MAX_ENTRIES; ++index)
    {
        char generated[] = "CFG.FULL.00";
        generated[9] = static_cast<char>('0' + ((index / 10U) % 10U));
        generated[10] = static_cast<char>('0' + (index % 10U));
        assertResult(ConfigResult::SUCCESS, fullRegistry.registerDefinition(
            ConfigDefinition::uint32Config(static_cast<ConfigId>(index + 1U), generated,
                static_cast<uint32_t>(index), 0U, UINT32_MAX, ConfigAccess::READ_WRITE)));
    }
    ConfigRuntime full(fullRegistry); assertResult(ConfigResult::SUCCESS, full.begin());
    TEST_ASSERT_EQUAL_UINT32(CONFIG_REGISTRY_MAX_ENTRIES - 1U,
        readUInt(full, static_cast<ConfigId>(CONFIG_REGISTRY_MAX_ENTRIES)));
    assertResult(ConfigResult::SUCCESS, full.setById(
        static_cast<ConfigId>(CONFIG_REGISTRY_MAX_ENTRIES), ConfigValue::fromUInt32(UINT32_MAX)));
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX,
        readUInt(full, static_cast<ConfigId>(CONFIG_REGISTRY_MAX_ENTRIES)));
}

static_assert(std::is_trivially_copyable<ConfigKey>::value, "ConfigKey must be value-copyable");
static_assert(std::is_trivially_copyable<ConfigValue>::value, "ConfigValue must be value-copyable");
static_assert(std::is_empty<ConfigValidator>::value, "Validator must remain stateless");

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_config_key_validation_boundaries_and_atomic_set);
    RUN_TEST(test_config_value_is_type_safe_bounded_and_binary_preserving);
    RUN_TEST(test_definition_factories_reject_invalid_defaults_and_constraints);
    RUN_TEST(test_validator_enforces_exact_type_and_inclusive_boundaries);
    RUN_TEST(test_registry_is_ordered_atomic_and_bounded);
    RUN_TEST(test_runtime_begin_get_set_reset_and_access_policies);
    RUN_TEST(test_write_once_locks_only_after_success_and_resets_cleanly);
    RUN_TEST(test_registry_mutation_is_detected_before_read_or_commit_and_rebegin_resyncs);
    RUN_TEST(test_empty_and_full_registry_runtime_boundaries);
    UNITY_END();
}

void loop() {}
