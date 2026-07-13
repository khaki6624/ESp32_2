#ifndef DEVICE_COMMON_H
#define DEVICE_COMMON_H

#include <stdint.h>
#include <math.h>

using DeviceId = uint16_t;
using DeviceTemplateId = uint16_t;
using NodeId = uint16_t;
using LocationId = uint16_t;

constexpr DeviceId INVALID_DEVICE_ID = 0;
constexpr DeviceTemplateId INVALID_DEVICE_TEMPLATE_ID = 0;
constexpr NodeId INVALID_NODE_ID = 0;
constexpr LocationId INVALID_LOCATION_ID = 0;

constexpr uint8_t DEVICE_TEMPLATE_NAME_MAX_LENGTH = 32;
constexpr uint8_t DEVICE_TEMPLATE_ICON_MAX_LENGTH = 24;
constexpr uint8_t DEVICE_NAME_MAX_LENGTH = 40;

enum class DriverType : uint8_t
{
    NONE = 0,
    RELAY,
    DIGITAL_INPUT,
    ANALOG_INPUT,
    IR,
    RF,
    GSM,
    ULTRASONIC,
    SOIL_MOISTURE,
    TEMPERATURE,
    HUMIDITY,
    DIMMER,
    ANALOG_OUTPUT
};

using DriverTypeMask = uint32_t;

constexpr uint8_t DRIVER_TYPE_COUNT = 13;
static_assert(DRIVER_TYPE_COUNT > 1U && DRIVER_TYPE_COUNT <= 32U, "Driver mask overflow");
constexpr DriverTypeMask ALL_KNOWN_DRIVER_TYPE_MASK =
    (static_cast<DriverTypeMask>(1U) << (DRIVER_TYPE_COUNT - 1U)) - 1U;

inline DriverTypeMask driverTypeToMask(DriverType type)
{
    const uint8_t value = static_cast<uint8_t>(type);
    return value > 0U && value < DRIVER_TYPE_COUNT
        ? static_cast<DriverTypeMask>(1U) << (value - 1U)
        : 0U;
}

inline bool hasDriverType(DriverTypeMask mask, DriverType type)
{
    const DriverTypeMask bit = driverTypeToMask(type);
    return bit != 0U && (mask & bit) != 0U;
}

inline void addDriverType(DriverTypeMask& mask, DriverType type)
{
    mask |= driverTypeToMask(type);
}

inline void removeDriverType(DriverTypeMask& mask, DriverType type)
{
    mask &= ~driverTypeToMask(type);
}

inline DriverTypeMask getAllKnownDriverTypeMask()
{
    return ALL_KNOWN_DRIVER_TYPE_MASK;
}

inline bool isValidDriverType(DriverType type)
{
    const uint8_t value = static_cast<uint8_t>(type);
    return value < DRIVER_TYPE_COUNT;
}

inline bool isValidDriverTypeMask(DriverTypeMask mask)
{
    return (mask & ~ALL_KNOWN_DRIVER_TYPE_MASK) == 0U;
}

enum class DeviceAction : uint8_t
{
    NONE = 0,
    READ,
    ON,
    OFF,
    TOGGLE,
    PULSE,
    TIMED_ON,
    OPEN,
    CLOSE,
    STOP,
    SET_POSITION,
    SET_LEVEL,
    INCREASE,
    DECREASE,
    ENABLE,
    DISABLE,
    RESET
};

using DeviceActionMask = uint32_t;

constexpr uint8_t DEVICE_ACTION_COUNT = 18;
static_assert(DEVICE_ACTION_COUNT > 1U && DEVICE_ACTION_COUNT <= 32U, "Action mask overflow");
constexpr DeviceActionMask ALL_KNOWN_ACTION_MASK =
    (static_cast<DeviceActionMask>(1U) << (DEVICE_ACTION_COUNT - 1U)) - 1U;

inline DeviceActionMask actionToMask(DeviceAction action)
{
    const uint8_t value = static_cast<uint8_t>(action);
    return value > 0U && value < DEVICE_ACTION_COUNT
        ? static_cast<DeviceActionMask>(1U) << (value - 1U)
        : 0U;
}

inline bool hasAction(DeviceActionMask mask, DeviceAction action)
{
    const DeviceActionMask bit = actionToMask(action);
    return bit != 0U && (mask & bit) != 0U;
}

inline void addAction(DeviceActionMask& mask, DeviceAction action)
{
    mask |= actionToMask(action);
}

inline void removeAction(DeviceActionMask& mask, DeviceAction action)
{
    mask &= ~actionToMask(action);
}

inline DeviceActionMask getAllKnownActionMask()
{
    return ALL_KNOWN_ACTION_MASK;
}

inline bool isValidAction(DeviceAction action)
{
    const uint8_t value = static_cast<uint8_t>(action);
    return value < DEVICE_ACTION_COUNT;
}

inline bool isValidActionMask(DeviceActionMask mask)
{
    return (mask & ~ALL_KNOWN_ACTION_MASK) == 0U;
}

enum class DeviceValueType : uint8_t
{
    NONE = 0,
    BOOLEAN,
    INTEGER,
    FLOAT,
    PERCENTAGE,
    ENUM_VALUE
};

constexpr uint8_t DEVICE_VALUE_TYPE_COUNT = 6;

inline bool isValidDeviceValueType(DeviceValueType type)
{
    return static_cast<uint8_t>(type) < DEVICE_VALUE_TYPE_COUNT;
}

struct DeviceValue
{
    DeviceValueType type;

    union
    {
        bool booleanValue;
        int32_t integerValue;
        float floatValue;
        uint8_t percentageValue;
        int32_t enumValue;
    };

    bool valid;
    uint32_t timestampMs;

    DeviceValue() :
        type(DeviceValueType::NONE),
        integerValue(0),
        valid(false),
        timestampMs(0)
    {
    }

    void invalidate(uint32_t timestamp = 0)
    {
        type = DeviceValueType::NONE;
        integerValue = 0;
        valid = false;
        timestampMs = timestamp;
    }

    void clear()
    {
        invalidate(0);
    }

    static DeviceValue invalid(uint32_t timestamp = 0)
    {
        DeviceValue result;
        result.invalidate(timestamp);
        return result;
    }

    static DeviceValue makeBoolean(bool value, uint32_t timestamp = 0)
    {
        DeviceValue result;
        result.type = DeviceValueType::BOOLEAN;
        result.booleanValue = value;
        result.valid = true;
        result.timestampMs = timestamp;
        return result;
    }

    static DeviceValue makeInteger(int32_t value, uint32_t timestamp = 0)
    {
        DeviceValue result;
        result.type = DeviceValueType::INTEGER;
        result.integerValue = value;
        result.valid = true;
        result.timestampMs = timestamp;
        return result;
    }

    static DeviceValue makeFloat(float value, uint32_t timestamp = 0)
    {
        if (!isfinite(value))
            return invalid(timestamp);

        DeviceValue result;
        result.type = DeviceValueType::FLOAT;
        result.floatValue = value;
        result.valid = true;
        result.timestampMs = timestamp;
        return result;
    }

    static DeviceValue makePercentage(int32_t value, uint32_t timestamp = 0)
    {
        if (value < 0 || value > 100)
            return invalid(timestamp);

        DeviceValue result;
        result.type = DeviceValueType::PERCENTAGE;
        result.percentageValue = static_cast<uint8_t>(value);
        result.valid = true;
        result.timestampMs = timestamp;
        return result;
    }

    static DeviceValue makeEnum(int32_t value, uint32_t timestamp = 0)
    {
        DeviceValue result;
        result.type = DeviceValueType::ENUM_VALUE;
        result.enumValue = value;
        result.valid = true;
        result.timestampMs = timestamp;
        return result;
    }

    bool getBoolean(bool& out) const
    {
        if (!valid || type != DeviceValueType::BOOLEAN)
            return false;
        out = booleanValue;
        return true;
    }

    bool getInteger(int32_t& out) const
    {
        if (!valid || type != DeviceValueType::INTEGER)
            return false;
        out = integerValue;
        return true;
    }

    bool getFloat(float& out) const
    {
        if (!valid || type != DeviceValueType::FLOAT)
            return false;
        out = floatValue;
        return true;
    }

    bool getPercentage(uint8_t& out) const
    {
        if (!valid || type != DeviceValueType::PERCENTAGE)
            return false;
        out = percentageValue;
        return true;
    }

    bool getEnum(int32_t& out) const
    {
        if (!valid || type != DeviceValueType::ENUM_VALUE)
            return false;
        out = enumValue;
        return true;
    }

    bool equals(const DeviceValue& other, float floatEpsilon = 0.0001f) const
    {
        if (valid != other.valid || type != other.type)
            return false;
        if (!valid)
            return true;

        switch (type)
        {
            case DeviceValueType::BOOLEAN:
                return booleanValue == other.booleanValue;
            case DeviceValueType::INTEGER:
                return integerValue == other.integerValue;
            case DeviceValueType::FLOAT:
            {
                // Epsilon منفی قدرمطلق می‌شود و مقدار غیرمتناهی به پیش‌فرض امن برمی‌گردد.
                const float epsilon = isfinite(floatEpsilon)
                    ? (floatEpsilon < 0.0f ? -floatEpsilon : floatEpsilon)
                    : 0.0001f;
                return fabsf(floatValue - other.floatValue) <= epsilon;
            }
            case DeviceValueType::PERCENTAGE:
                return percentageValue == other.percentageValue;
            case DeviceValueType::ENUM_VALUE:
                return enumValue == other.enumValue;
            case DeviceValueType::NONE:
            default:
                return false;
        }
    }
};

// Arduino Core نام عمومی DISABLED را Macro کرده است؛ قرارداد Enum باید همین نام را حفظ کند.
#ifdef DISABLED
#undef DISABLED
#endif

enum class DeviceHealth : uint8_t
{
    UNKNOWN = 0,
    OK,
    OFFLINE,
    DEGRADED,
    ERROR,
    DISABLED
};

struct DeviceBinding
{
    NodeId nodeId = INVALID_NODE_ID;
    DriverType driverType = DriverType::NONE;
    uint8_t driverInstance = 0;
    uint8_t channel = 0;
    bool valid = false;

    bool isValid() const
    {
        return valid && nodeId != INVALID_NODE_ID && driverType != DriverType::NONE &&
               isValidDriverType(driverType);
    }

    bool operator==(const DeviceBinding& other) const
    {
        // Equality فقط آدرس فیزیکی Binding را مقایسه می‌کند، نه Flag اعتبار را.
        return nodeId == other.nodeId && driverType == other.driverType &&
               driverInstance == other.driverInstance && channel == other.channel;
    }

    bool operator!=(const DeviceBinding& other) const
    {
        return !(*this == other);
    }
};

#endif
