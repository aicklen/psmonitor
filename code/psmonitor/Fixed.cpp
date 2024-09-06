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

//    const int shift PROGMEM = 5;
    const fixed fixed_one = int2fixed(1) << shift;

    fixed int2fixed(int16_t value){
        return ((fixed) value) << shift;
    }

    int16_t fixed2int(fixed value){
        return (int16_t)(value >> shift);
    }

    fixed multiply(fixed value1, fixed value2){
        int64_t iv = (int64_t)value1 * (int64_t)value2;
        return (fixed)(iv >> shift);
    }

    fixed invert(fixed value){
        int64_t iv = ((int64_t)fixed_one << shift) / (int64_t)value;
        return (fixed)iv;
    }

}
