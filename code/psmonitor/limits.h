#pragma once
/**
 * @file limits.h
 * @copyright
 * Copyright 2024 Gregory Aicklen.
 * Licensed under the MIT License
 *
 * @brief Header file defining maximum specs for a monitored power supply.
 *
 */
 
#ifndef _LIMITS_H
#define _LIMITS_H

// Limits (both supplies have symmetric specs)
enum LIMITS : int16_t {
    LIMIT_MAX_VOLTAGE = 15000,  // Voltage limits in millivolts
    LIMIT_MAX_CURRENT = 1000,   // Current limit in milliamps
};
#endif
