#ifndef DEVICE_IDENTITY_H
#define DEVICE_IDENTITY_H

#include <Arduino.h>
#include <DeviceCommon.h>

namespace DeviceIdentity
{
    //==================================================
    // Device Information
    //==================================================

    constexpr char DEVICE_NAME[] = "Central Controller";

    constexpr char HARDWARE_VERSION[] = "REV_A";

    constexpr char FIRMWARE_FAMILY[] = "SmartHome";

    //==================================================
    // Default Parameters
    //==================================================

    constexpr NodeId DEFAULT_NODE_ID = 1;

    constexpr uint8_t DEFAULT_MODBUS_ADDRESS = 1;
}

#endif
