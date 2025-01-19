#pragma once
/**
 * @file globals.h
 * @copyright
 * Copyright 2024 Gregory Aicklen.
 * Licensed under the MIT License
 *
 * @brief Header file declaring global storage.
 *
 * Declares external storage shared between tasks (oh, the things we do to
 * conserve precious memory)
 *
 */

#ifndef _GLOBALS_H
#define _GLOBALS_H

/// Array of integers holding a set of current and voltage measurements
extern int16_t readings[4];

/// Buffer used to construct strings displayed on LCD monitor
extern char string_buf[17];

#endif
