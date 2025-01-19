/**
 * @file Calibration.cpp
 * @copyright
 * Copyright 2024 Gregory Aicklen.
 * Licensed under the MIT License
 *
 * @brief Stores and retrieves calibration data to/from EEPROM.
 *
 * Implements reading and writing calibration data to/from EEPROM, including checking/computing
 * a CRC for the data to ensure that unwritten or corrupted calibration data are detected and
 * rejected. 
 *
 * If calibration not performed or data corrupt, the following defaults are used for
 * all channels:
 *      - Voltage Offset: 0
 *      - Voltage Gain: 1
 *      - Current Offset: 0
 *      - Current Gain: 1
 */

// Standard header files
#include <Arduino.h>
#include <EEPROM.h>
#include <CRC.h>

// Include our own header file
#include "Calibration.h"

// External storage shared between tasks
#include "globals.h"

// Limits on power supply output voltages and currents
#include "limits.h"

// MonitorTask header to include MONITOR_* enums
#include "MonitorTask.h"

// Fixed point arithmetic specific to this project
#include "Fixed.h"

// Array of integers holding a set of current and voltage measurements
extern int16_t readings[4];


namespace Calibration {

    // A common struct for calibration data; change once and it changes eerywhere.
    struct cal_data {
        int16_t offsets[4];    // Array of voltage and current offset corrections
        fixed gains[4];        // Fixed point array of voltage and current gain correction factors
        uint16_t crc;          // CRC value validating calibration data
    };

    // EEPROM struct containing calibration data
    cal_data eeprom_data;

    // Default calibration data
    cal_data calibration_defaults = {
        {0, 0, 0, 0},
        {Fixed::int2fixed(1), Fixed::int2fixed(1), Fixed::int2fixed(1), Fixed::int2fixed(1)},
        calcCRC16(reinterpret_cast<uint8_t*>(&calibration_defaults), sizeof(calibration_defaults) - sizeof(calibration_defaults.crc))
    };

    // Points to the working copy of current calibration data
    cal_data *calibration_data = &calibration_defaults;


    // Manage recall of persistent calibration data
    auto data_recalled = bool{false};
    auto data_valid = bool{false};


    /**
    * @brief Returns status of calibration data
    *
    * Returns logical and of data_recalled and data_valid, indicating overall validity of
    * calibration data and whether default values or calibrated values are being used.
    */
    bool calibrated(void) {
        return data_recalled && data_valid;
    }


    /**
    * @brief Retrieves calibration values from EEPROM and populates calibration data arrays
    *
    * Retrieves persistent calibration data from EEPROM and determines if values are valid
    * (by checking CRC). If valid, calibration data[] is populated, else default values
    * of zero offset and unity gain are used.
    */
    void recall(void) {
        EEPROM.get(CALIBRATION_DATA_ADDRESS, eeprom_data);
        data_recalled = true;
        uint16_t check_crc = calcCRC16(reinterpret_cast<uint8_t*>(&eeprom_data), sizeof(eeprom_data) - sizeof(eeprom_data.crc));
        if (eeprom_data.crc == check_crc) {
            data_valid = true;
            calibration_data = &eeprom_data;
            return;
        }

        // If data hasn't been set or has been corrupted, use the default data
        data_valid = false;
        calibration_data = &calibration_defaults;
        return;
    }


    /**
    * @brief Computes calibration values from readings in global memory
    *
    * Computes calibration data from actual readings (i.e. the values that the calibrating
    * technician was *supposed* to set) and the measured readings. Computes CRC and updates
    * EEPROM with the new values.
    *
    * @param actuals      Array of voltage and current readings containing the
    *                     actual values that *should* have been set by the technician
    *                     during the calibration procedure.
    * @param measured     Array of voltage and current readings containing the
    *                     measured values taken during the calibration procedure.
    */
    void update(int16_t actuals[], int16_t measured[]) {
        // Perform computations here; does nothing for now
        return;
    }


    /**
    * @brief Corrects values for offset and gain error using calibration data
    *
    * Computes calibration data from actual readings (i.e. the values that the calibrating
    * technician was *supposed* to set) and the measured readings. Computes CRC and updates
    * EEPROM with the new values.
    *
    * @param param_id     Identifies specific parameter is being corrected. Use DATA_SELECT_VALUE
    *                     enum defined in Calibration.h.
    *
    * @param value        Voltage or current to be corrected.
    */

    int16_t correct(const int16_t param_id, const int16_t value){
        // Does nothing for now
        return value;
    }

}