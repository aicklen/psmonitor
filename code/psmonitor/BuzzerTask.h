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
#define BEEP_BLIP 50
#define BEEP_SHORT 100
#define BEEP_MEDIUM 250
#define BEEP_LONG 500
#define BEEP_SPACING 50

namespace BuzzerTask {

    // Functions
    void setup(uint8_t pin,
               uint8_t on_value,
               uint8_t off_value);
    void beep(unsigned duration, unsigned count);
    void mute(void);
    void unmute(void);
    void toggleMute(void);
    void update(void);
};

#endif