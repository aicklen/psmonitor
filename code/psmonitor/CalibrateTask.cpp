/**
 * @file CalibrateTask.cpp
 * @copyright
 * Copyright 2024 Gregory Aicklen.
 * Licensed under the MIT License
 *
 * @brief Implements a CalibrateTask that determines gain and offset errors for all voltages and currents.
 *
 * Implements a CalibrateTask that measures sets of raw values from the ADCs of two INA260
 * voltage/current sensors and computes gain and offset corrections for all measurements.
 * Corrections are written to EEPROM where they can be used by other tasks.
 *
 * To use the Calibrate task:
 *      - CalibrateTask::setup() - setup the task
 *      - CalibrateTask::update() - run the calibration process; call once each time through scheduler.
 *        Waits for button presses between steps in the process
 *      - CalibrateTask::buttonPress() - call to advance to the next step in the process
 *      - CalibrateTask::finished() - call to determine if calibration process is finished.
 *        Scheduling of the Calibrate task should stop when this flag becomes True
 *
 * Intended to be run by a simple, non-preemptive round robin scheduler, where each task
 * is responsible for relinquishing control; this code is *not threadsafe* and must not be
 * interrupted.
 *
 * @todo Actually implement the calibration process, create calibration data, and write to EEPROM
 * @todo Implement a mechanism to abort this task without changing current calibration data
 */

// Include standard headers as needed
#include <Arduino.h>

// Include headers for third party libraries
#include "Adafruit_INA260.h"
#include <LiquidCrystal.h>
//#include <EEPROM.h>

// Include our own header file and supporting utilities
#include "CalibrateTask.h"
#include "Calibration.h"

// Include headers for other tasks that are used during calibration
#include "MonitorTask.h"

// External storage shared between tasks
#include "globals.h"

// Limits on power supply output voltages and currents
#include "limits.h"

namespace CalibrateTask {

    /// Define values for the various states of the calibration state machine
    typedef enum _state {
        CALIBRATE_INITIALIZE = 0,
        CALIBRATE_PROMPT_LOW_V = 1,
        CALIBRATE_READ_LOW_V_PROMPT_HIGH_V = 2,
        CALIBRATE_READ_HIGH_V_PROMPT_LOW_I = 3,
        CALIBRATE_READ_LOW_I_PROMPT_HIGH_I = 4,
        CALIBRATE_READ_HIGH_I_FINISH = 5,
    } CALIBRATE_STATE;

    // Forward declarations as needed
    char* generateVoltageString(int16_t value);
    void updateCalibrationData(int measurement_type, int position, int16_t actual);

    // Initialize additional variables
    bool finishedFlag = false;
    bool executeStep = true;
    uint8_t currentState = CALIBRATE_INITIALIZE;
    LiquidCrystal *lcd = nullptr;
    // int16_t rawPoints[4];     // (x1, y1), (x2, y2)

    /// Create required plus/minus special character for lcd
    byte chr1[8] =  { 
                    B00100,
                    B00100,
                    B11111,
                    B00100,
                    B00100,
                    B00000,
                    B11111,
                    B00000
                    };  

    // Define the prompt strings used, store 
    const char promptIntro[] PROGMEM = "  Calibration";
    const char promptPushButton[] PROGMEM = "  Push Button";
    const char promptVolts[] PROGMEM = "Volts:  ";
    const char promptCurrent[] PROGMEM = "mAmps:  ";

    // Calibration target values
    const int CALIBRATE_LOW_V = LIMIT_MAX_VOLTAGE / 10;
    const int CALIBRATE_HIGH_V = LIMIT_MAX_VOLTAGE / 10 * 9;
    const int CALIBRATE_LOW_I = LIMIT_MAX_CURRENT / 10;
    const int CALIBRATE_HIGH_I = LIMIT_MAX_CURRENT / 10 * 9;

    // Raw calibration data sent to Calibration utility
    int16_t actuals[8];     // Actual values that *should* have benn set by technician
    int16_t measured[8];    // Raw measured values corresponding to actuals

    /**
    * @brief Configures the calibrate task LCD display and loads special characters into display.
    *
    * @param display   Pointer to the LCD display object
    */
    void setup(LiquidCrystal *display) {
        // Save the pointer to the lcd display
        lcd = display;

        // Load required special character into lcd 
        lcd->createChar(1, chr1);
    }

    /**
    * @brief Processes notification that "the button" was pressed; permits execution of next step.
    *
    */
    void buttonPress(void) {
        executeStep = true;
    }

    /**
    * @brief Returns the value of the *finished* flag indicating that the calibration task has completed.
    *
    * @return value of the *finished* flag indicating that the calibration task has completed
    */
    bool finished(void) {
        return finishedFlag;
    }

    /**
    * @brief Called each time through the scheduling loop to implement calibration.
    *
    * Implements the CalibrateTask. Runs whenever the scheduler transfers control to the
    * CalibrateTask and determines if it is time to run; if not time to run, immediately
    * relinquishes control back to the scheduler.
    */
    void update(void) {
        if (!finishedFlag && executeStep) {
            executeStep = false;    // Wait for a button push to run next step
            lcd->clear();
            lcd->setCursor(0, 0);

            // Perform calibration routine state machine
            switch (currentState) {
            case CALIBRATE_INITIALIZE:
                lcd->print((const __FlashStringHelper *)promptIntro);
                lcd->setCursor(0, 1);
                lcd->print((const __FlashStringHelper *)promptPushButton);
                currentState = CALIBRATE_PROMPT_LOW_V;
                break;

            case CALIBRATE_PROMPT_LOW_V:
                lcd->print((const __FlashStringHelper *)promptVolts);
                lcd->write(1);
                lcd->print(generateVoltageString(CALIBRATE_LOW_V));
                lcd->setCursor(0, 1);
                lcd->print((const __FlashStringHelper *)promptPushButton);
                currentState = CALIBRATE_READ_LOW_V_PROMPT_HIGH_V;
                break;

            case CALIBRATE_READ_LOW_V_PROMPT_HIGH_V:
                MonitorTask::getRawValues();
                updateCalibrationData(MONITOR_VOLTAGE, MEASURED_LOW_V, CALIBRATE_LOW_V);
                lcd->print((const __FlashStringHelper *)promptVolts);
                lcd->write(1);
                lcd->print(generateVoltageString(CALIBRATE_HIGH_V));
                lcd->setCursor(0, 1);
                lcd->print((const __FlashStringHelper *)promptPushButton);
                currentState = CALIBRATE_READ_HIGH_V_PROMPT_LOW_I;
                break;

            case CALIBRATE_READ_HIGH_V_PROMPT_LOW_I:
                MonitorTask::getRawValues();
                updateCalibrationData(MONITOR_VOLTAGE, MEASURED_HIGH_V, CALIBRATE_HIGH_V);
                lcd->print((const __FlashStringHelper *)promptCurrent);
                (void) sprintf(string_buf, "% 5d", CALIBRATE_LOW_I);
                lcd->print(string_buf);
                lcd->print(".0");
                lcd->setCursor(0, 1);
                lcd->print((const __FlashStringHelper *)promptPushButton);
                currentState = CALIBRATE_READ_LOW_I_PROMPT_HIGH_I;
                break;

            case CALIBRATE_READ_LOW_I_PROMPT_HIGH_I:
                MonitorTask::getRawValues();
                updateCalibrationData(MONITOR_CURRENT, MEASURED_LOW_I, CALIBRATE_LOW_I);
                lcd->print((const __FlashStringHelper *)promptCurrent);
                (void) sprintf(string_buf, "% 5d", CALIBRATE_HIGH_I);
                lcd->print(string_buf);
                lcd->print(".0");
                lcd->setCursor(0, 1);
                lcd->print((const __FlashStringHelper *)promptPushButton);
                currentState = CALIBRATE_READ_HIGH_I_FINISH;
                break;

            case CALIBRATE_READ_HIGH_I_FINISH:
            default:
                MonitorTask::getRawValues();
                updateCalibrationData(MONITOR_CURRENT, MEASURED_HIGH_I, CALIBRATE_HIGH_I);
                Calibration::update(actuals, measured);
                finishedFlag = true;
            }
        }
    }

    /**
    * @brief Generates a voltage string from the provided integer value.
    *
    * Generate voltage string from provided value. Takes value in millivolts and presents as decimal
    * volts with precision to thousandths of a volt.
    *
    * @param value   Integer value of voltage in millivolts
    *
    * @return String representing decimal volts to thousandths of a volt
    */
    char* generateVoltageString(int16_t value) {
        // Generate the string from integer values
        (void) sprintf(string_buf, "%5d", value);
        // Set seventh character to nul to terminate string
        string_buf[6] = 0;
        // Move rightmost 3 digits to the right. Not worth a loop.
        string_buf[5] = string_buf[4];
        string_buf[4] = string_buf[3];
        string_buf[3] = string_buf[2];
        // Add decimal point
        string_buf[2] = '.';
        return string_buf;
    }

    /**
    * @brief Populates subset of calibration data array.
    *
    * Populates subset of calibration data array with data acquired during
    * each step of the calibration process.
    *
    * @param measurement_type   Which parameter, voltage or current, was measured
    * @param position   Which voltage or current was measured (high or low)
    * @param actual     Voltage or current that technician *should* have set for this step
    */
    void updateCalibrationData(int measurement_type, int position, int16_t actual) {
        measured[MEASURED_POS + position] = readings[measurement_type + MONITOR_POS];
        measured[MEASURED_NEG + position] = readings[measurement_type + MONITOR_NEG];
        actuals[MEASURED_POS + position] = actual;
        actuals[MEASURED_NEG + position] = actual;
    }

}