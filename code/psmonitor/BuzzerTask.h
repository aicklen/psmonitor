#pragma once
/**
 * @file BuzzerTask.h
 * @copyright
 * Copyright 2024 Gregory Aicklen.
 * Licensed under the MIT License
 *
 * @brief Header file for BuzzerTask.
 *
 */

#ifndef _BUZZERTASK_H
#define _BUZZERTASK_H

// Include standard headers as needed
#include <Arduino.h>

// Some predefined buzzer durations
enum BEEP : uint32_t {
    BEEP_BLIP = 50,
    BEEP_SHORT = 100,
    BEEP_MEDIUM = 250,
    BEEP_LONG = 500,
    BEEP_SPACING = 50
};

namespace BuzzerTask {

    // Functions
    void setup(const uint8_t pin,
               const uint8_t on_value,
               const uint8_t off_value);
    void beep(const uint32_t duration, const uint32_t count);
    void mute(void);
    void unmute(void);
    void toggleMute(void);
    void update(void);
};

#endif