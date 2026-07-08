#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <Arduino.h>

namespace BoardConfig
{
    struct OutputConfig
    {
        uint8_t gpio;
        bool activeLow;
    };

    // ===== Board Information =====

    constexpr char BOARD_NAME[] = "Central Controller";

    constexpr uint8_t BOARD_ID = 1;

    constexpr bool HAS_WIFI = true;
    constexpr bool HAS_RS485 = true;
    constexpr bool HAS_RF = true;
    constexpr bool HAS_IR = true;

    // ===== Outputs =====

    constexpr uint8_t OUTPUT_COUNT = 16;

    constexpr OutputConfig outputs[OUTPUT_COUNT] =
    {
        {23, true},
        {22, true},
        {21, true},
        {19, true},
        {18, true},
        {17, true},
        {16, true},
        {15, true},

        {14, true},
        {13, true},
        {12, true},
        {27, true},
        {26, true},
        {25, true},
        {33, true},
        {32, true}
    };

}

#endif