/**
 * @file CalibrationData.h
 * @copyright
 * Copyright 2024 Gregory Aicklen.
 * Licensed under the MIT License
 *
 * @brief Header file defining data structure for calibration data.
 *
 */
 
#ifndef _CALIBRATION_H
#define _CALIBRATION_H


// Offsets into measured values in actuals[] and measured[] arrays
typedef enum _measured_slots {
    MEASURED_LOW_V =  0,    // For any value in measured[], low voltage measurement
    MEASURED_HIGH_V = 1,    // For any value in measured[], high voltage measurement
    MEASURED_LOW_I =  2,    // For any value in measured[], low current measurement
    MEASURED_HIGH_I = 3,    // For any value in measured[], high current measurement
} MEASURED_SELECT_SLOT;

// Calibration of positive and negative voltages use two sets of data
// contained in actuals[] and measured[]
typedef enum _measured_pos_neg {
    MEASURED_POS = 0,
    MEASURED_NEG = 4,
} MEASURED_SELECT_POS_NEG;

// Indexes into calibration data[] array; used to identify which parameter is being corrected
typedef enum _data_select {
    DATA_VOLTAGE_POS = 0,    // Index of positive voltage entries
    DATA_VOLTAGE_NEG = 1,    // Index of negative voltage entries
    DATA_CURRENT_POS = 2,    // Index of positive current entries
    DATA_CURRENT_NEG = 3,    // Index of negative current entries
} DATA_SELECT_VALUE;


namespace Calibration {

    const int16_t CALIBRATION_DATA_ADDRESS = 0;

    bool calibrated(void);
    void recall(void);
    void update(int16_t actuals[], int16_t measured[]);

    int16_t correct(int param_id, int16_t value);

}

#endif