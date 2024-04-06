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

/// Voltage limits in millivolts (dual supplies have symmetric specs)
#define LIMIT_MAX_VOLTAGE 15000
 
/// Current limits in milliamps (dual supplies can source same current)
#define LIMIT_MAX_CURRENT 1000
 
#endif
