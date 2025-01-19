/**
 * @file Fixed.cpp
 * @copyright
 * Copyright 2024 Gregory Aicklen.
 * Licensed under the MIT License
 *
 * @brief Supports fixed point arithmetic required in this application.
 *
 */

// Standard header files
#include <Arduino.h>

// Include our own header file
#include "Fixed.h"

// External storage shared between tasks
#include "globals.h"

// Limits on power supply output voltages and currents
#include "limits.h"

namespace Fixed {

    const int16_t shift PROGMEM = 5;

    fixed int2fixed(const int16_t value){
        return static_cast<fixed>(value << shift);
    }

    fixed fixed_one = int2fixed(1) << shift;

    int16_t fixed2int(const fixed value){
        return (int16_t)(value >> shift);
    }

    fixed multiply(const fixed value1, const fixed value2){
        return static_cast<fixed>((static_cast<int64_t>(value1) * static_cast<int64_t>(value2)) >> shift);
    }

    fixed invert(const fixed value){
        return static_cast<fixed>(((int64_t)fixed_one << shift) / (int64_t)value);
    }

}
