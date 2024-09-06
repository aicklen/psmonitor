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
typedef enum _select {
    MONITOR_VOLTAGE_POS = 0,    /// Index of positive voltage reading
    MONITOR_VOLTAGE_NEG = 1,    /// Index of negative voltage reading
    MONITOR_CURRENT_POS = 2,    /// Index of positive current reading
    MONITOR_CURRENT_NEG = 3,    /// Index of negative current reading
} MONITOR_SELECT_VALUE;

// Relative indexes into readings[] returned by Monitor task (measurement type)
typedef enum _select_type {
    MONITOR_VOLTAGE = 0,    /// Index of positive voltage reading
    MONITOR_CURRENT = 2,    /// Index of positive current reading
} MONITOR_SELECT_TYPE;

// Relative indexes into measurement types in readings[] returned by Monitor task
typedef enum _select_sign {
    MONITOR_POS = 0,    /// Index of positive reading
    MONITOR_NEG = 1,    /// Index of negative reading
} MONITOR_SELECT_SIGN;

namespace MonitorTask {

    void setup(unsigned long interval,
                           uint8_t pos_addr,
                           uint8_t neg_addr,
                           LiquidCrystal *display);

    // Normal methods
    bool communicationOK(void);
    void setAveragingCount(INA260_AveragingCount count);
    void setConversionTime(INA260_ConversionTime conv);
    void update(void);
    void getRawValues(void);

}

#endif
