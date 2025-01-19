#pragma once
/**
 * @file CalibrateTask.h
 * @copyright
 * Copyright 2024 Gregory Aicklen.
 * Licensed under the MIT License
 *
 * @brief Header file for CalibrateTask.
 *
 */

#ifndef _CALIBRATETASK_H
#define _CALIBRATETASK_H

// Include headers for third party libraries
#include <LiquidCrystal.h>

namespace CalibrateTask {

    void setup(LiquidCrystal *display);
    void buttonPress(void);
    bool finished(void);
    void update(void);

}

#endif
