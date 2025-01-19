#pragma once
/**
 * @file MonitorTask.h
 * @copyright
 * Copyright 2024 Gregory Aicklen.
 * Licensed under the MIT License
 *
 * @brief Header file for MonitorTask.
 *
 */

#ifndef _MONITORTASK_H
#define _MONITORTASK_H

// Include third party libraries
#include "Adafruit_INA260.h"
#include <LiquidCrystal.h>

// Indexes into readings[] returned by Monitor task.
enum MONITOR_SELECT_VALUE : int16_t {
    MONITOR_VOLTAGE_POS = 0,
    MONITOR_VOLTAGE_NEG = 1,
    MONITOR_CURRENT_POS = 2,
    MONITOR_CURRENT_NEG = 3,
};

// Relative indexes into readings[] grouped by measurement type
enum MONITOR_SELECT_GROUP : int16_t {
    MONITOR_VOLTAGE = 0,
    MONITOR_CURRENT = 2,
};

// Relative indexes into readings[] groups
enum MONITOR_SELECT_SIGN : int16_t {
    MONITOR_POS = 0,
    MONITOR_NEG = 1,
};

namespace MonitorTask {

    void setup(const uint32_t interval,
               const uint8_t pos_addr,
               const uint8_t neg_addr,
               LiquidCrystal *display);

    // Normal methods
    bool communicationOK(void);
    void setAveragingCount(INA260_AveragingCount count);
    void setConversionTime(INA260_ConversionTime conv);
    void update(void);
    void getRawValues(void);

}

#endif
