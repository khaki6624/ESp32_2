#include "ConfigValidator.h"

namespace
{
size_t validatedStringLength(const char* value)
{
    size_t result = 0U;
    while (result < CONFIG_STRING_MAX_LENGTH && value[result] != '\0') ++result;
    return result;
}
}

ConfigResult ConfigValidator::validate(const ConfigDefinition& definition,
    const ConfigValue& value) const
{
    if (!definition.isValid()) return ConfigResult::INVALID_VALUE;
    if (!value.isValid()) return ConfigResult::INVALID_VALUE;
    if (value.type() != definition.type()) return ConfigResult::TYPE_MISMATCH;

    if (value.type() == ConfigValueType::BOOLEAN) return ConfigResult::SUCCESS;
    if (value.type() == ConfigValueType::UINT32)
    {
        uint32_t candidate = 0U;
        if (!value.getUInt32(candidate)) return ConfigResult::INTERNAL_ERROR;
        return candidate >= definition.uint32Minimum() &&
               candidate <= definition.uint32Maximum()
            ? ConfigResult::SUCCESS : ConfigResult::INVALID_VALUE;
    }
    if (value.type() == ConfigValueType::INT32)
    {
        int32_t candidate = 0;
        if (!value.getInt32(candidate)) return ConfigResult::INTERNAL_ERROR;
        return candidate >= definition.int32Minimum() &&
               candidate <= definition.int32Maximum()
            ? ConfigResult::SUCCESS : ConfigResult::INVALID_VALUE;
    }
    const char* candidate = nullptr;
    if (!value.getString(candidate)) return ConfigResult::INTERNAL_ERROR;
    const size_t length = validatedStringLength(candidate);
    return length < CONFIG_STRING_MAX_LENGTH &&
           length <= definition.stringMaximumLength() &&
           (length == 0U ? definition.stringAllowsEmpty()
                         : length >= definition.stringMinimumLength())
        ? ConfigResult::SUCCESS : ConfigResult::INVALID_VALUE;
}
