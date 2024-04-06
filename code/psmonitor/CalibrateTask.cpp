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

// Include our own header file
#include "CalibrateTask.h"

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
        CALIBRATE_PROMPT_V1 = 1,
        CALIBRATE_READ_V1_PROMPT_V2 = 2,
        CALIBRATE_READ_V2_PROMPT_I1 = 3,
        CALIBRATE_READ_I1_PROMPT_I2 = 4,
        CALIBRATE_READ_I2_FINISH = 5,
    } CALIBRATE_STATE;

    /// Define indexes into vector of raw data points
    typedef enum _index {
        CALIBRATE_X1 = 0,    // Index of raw x1 reading
        CALIBRATE_Y1 = 1,    // Index of raw y1 reading
        CALIBRATE_X2 = 2,    // Index of raw x2 reading
        CALIBRATE_Y2 = 3,    // Index of raw y2 reading
    } CALIBRATE_INDEX;

    // Forward declare functions as needed
    char* generateVoltageString(int16_t value);

    // Initialize additional variables
    bool finishedFlag = false;
    bool executeStep = true;
    uint8_t currentState = CALIBRATE_INITIALIZE;
    LiquidCrystal *lcd = nullptr;
    int16_t rawPoints[4];     // (x1, y1), (x2, y2)

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

    // Calibration values
    const int CALIBRATE_V1 = LIMIT_MAX_VOLTAGE / 10;
    const int CALIBRATE_V2 = LIMIT_MAX_VOLTAGE / 10 * 9;
    const int CALIBRATE_I1 = LIMIT_MAX_CURRENT / 10;
    const int CALIBRATE_I2 = LIMIT_MAX_CURRENT / 10 * 9;

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
    * @brief Processes notification that "the" button was pressed; permits execution of next step.
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
                currentState = CALIBRATE_PROMPT_V1;
                break;

            case CALIBRATE_PROMPT_V1:
                lcd->print((const __FlashStringHelper *)promptVolts);
                lcd->write(1);
                lcd->print(generateVoltageString(CALIBRATE_V1));
                lcd->setCursor(0, 1);
                lcd->print((const __FlashStringHelper *)promptPushButton);
                currentState = CALIBRATE_READ_V1_PROMPT_V2;
                break;

            case CALIBRATE_READ_V1_PROMPT_V2:
                MonitorTask::getRawValues();
                lcd->print((const __FlashStringHelper *)promptVolts);
                lcd->write(1);
                lcd->print(generateVoltageString(CALIBRATE_V2));
                lcd->setCursor(0, 1);
                lcd->print((const __FlashStringHelper *)promptPushButton);
                currentState = CALIBRATE_READ_V2_PROMPT_I1;
                break;

            case CALIBRATE_READ_V2_PROMPT_I1:
                MonitorTask::getRawValues();
                lcd->print((const __FlashStringHelper *)promptCurrent);
                (void) sprintf(string_buf, "% 5d", CALIBRATE_I1);
                lcd->print(string_buf);
                lcd->print(".0");
                lcd->setCursor(0, 1);
                lcd->print((const __FlashStringHelper *)promptPushButton);
                currentState = CALIBRATE_READ_I1_PROMPT_I2;
                break;

            case CALIBRATE_READ_I1_PROMPT_I2:
                MonitorTask::getRawValues();
                lcd->print((const __FlashStringHelper *)promptCurrent);
                (void) sprintf(string_buf, "% 5d", CALIBRATE_I2);
                lcd->print(string_buf);
                lcd->print(".0");
                lcd->setCursor(0, 1);
                lcd->print((const __FlashStringHelper *)promptPushButton);
                currentState = CALIBRATE_READ_I2_FINISH;
                break;

            case CALIBRATE_READ_I2_FINISH:
            default:
                MonitorTask::getRawValues();
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

}