/**
 * @file MonitorTask.cpp
 * @copyright
 * Copyright 2024 Gregory Aicklen.
 * Licensed under the MIT License
 *
 * @brief Implements a MonitorTask that reads voltage and current from INA260 sensors and displays on LCD.
 *
 * Implements a MonitorTask that measures sets of voltage and current values from the ADCs of two INA260
 * voltage/current sensors, corrects for gain and offset errors using corrections stored in EEPROM,
 * and displays both positive and negative supply voltages and currents on the LCD display.
 *
 * To use the Monitor task:
 *      - MonitorTask::setup() - setup the basic task parameters and establish connection to INA260 sensors
 *      - MonitorTask::communicationOK() - check that communication with INA260s has been established
 *      - MonitorTask::setAveragingCount() - set number of samples taken and averaged for each measurement; optional
 *      - MonitorTask::setConversionTime() - set ADC conversion time per sample for each measurement; optional
 *      - MonitorTask::update() - run the monitor task; call once each time through scheduler
 *      - MonitorTask::getRawValues() - call anytime after setup to retrieve raw, unscaled and uncorrected values from sensors
 *
 * Intended to be run by a simple, non-preemptive round robin scheduler, where each task
 * is responsible for relinquishing control; this code is *not threadsafe* and must not be
 * interrupted.
 *
 * @todo Actually use calibration data from EEPROM to correct readings
 */

// Include standard headers as needed
#include <Arduino.h>

// Include third party libraries
#include "Adafruit_INA260.h"

// Include our own header file
#include "MonitorTask.h"

// Include other tasks that are part of this application
#include "BuzzerTask.h"

// When any output exceeds the design range, beep once for every
// OVER_RANGE_BEEP_N times the Monitor task runs.
#define OVER_RANGE_BEEP_N 10

// External storage shared between tasks
#include "globals.h"

// Limits on power supply output voltages and currents
#include "limits.h"

// Storage indexed to readings
int16_t readings[4];
//bool alert[4];

namespace MonitorTask {

    // Forward declarations
    int16_t nearest10(int16_t value);
    char* generateVoltageString(int16_t value);

    bool commOKFlag = false;
    bool startedFlag = false;
    
    // Task timing (all in milliseconds)
    unsigned long taskInterval = 250;
    unsigned long rolloverThreshold = 2500;  // Should be 10*taskInterval
    unsigned long currentTime;
    unsigned long targetTime = 0;

    // Buzzer management when alerting on over spec usage
    uint8_t beep_count = 0;
    
    // Classes to manage INA260 voltage/current monitoring devices
    Adafruit_INA260 ina260Pos = Adafruit_INA260();
    Adafruit_INA260 ina260Neg = Adafruit_INA260();

    // Alert limit flags
    bool alert[4];

    // Display LCD
    LiquidCrystal *lcd;


    /**
    * @brief Configures the Monitor task basic operating parameters and LCD display.
    *
    * @param interval   Time in milliseconds between task runs
    * @param pos_addr   I2C address of positive voltage/current INA260 sensor
    * @param neg_addr   I2C address of negative voltage/current INA260 sensor
    * @param display    Pointer to the LCD display object
    *
    * @todo Retrieve calibration data from EEPROM
    */
    void setup(unsigned long interval,
               uint8_t pos_addr,
               uint8_t neg_addr,
               LiquidCrystal *display) {

        // This initialization only performed once
        // Initialize state variables
        taskInterval = interval;
        rolloverThreshold = 100 * taskInterval;

        // Initialize and verify communication with the ina260 devices
        if (ina260Pos.begin(pos_addr) && ina260Neg.begin(neg_addr)) {
            commOKFlag = true;
        }

        // Save the display
        lcd = display;

        return;
    }

    /**
    * @brief Gets the current state of communications with the INA260 sensors.
    *
    * @return State of communication with INA260 sensors
    */
    bool communicationOK() {
        return commOKFlag;
    }

    /**
    * @brief Set the number of samples to average.
    *
    */
    void setAveragingCount(INA260_AveragingCount count) {
        ina260Pos.setAveragingCount(count);
        ina260Neg.setAveragingCount(count);
    }

    /**
    * @brief Set the time over which to measure the current and bus voltage samples.
    *
    */
    void setConversionTime(INA260_ConversionTime conv) {
        ina260Pos.setVoltageConversionTime(conv);
        ina260Pos.setCurrentConversionTime(conv);
        ina260Neg.setVoltageConversionTime(conv);
        ina260Neg.setCurrentConversionTime(conv);
    }

    /**
    * @brief Called each time through the scheduling loop to implement monitoring.
    *
    * Implements the MonitorTask main function to display current and voltage for both
    * positive and negative supplies. Runs whenever the scheduler transfers control to the
    * MonitorTask and determines if it is time to run; if not time to run, immediately
    * relinquishes control back to the scheduler.
    */
    void update() {
        currentTime = millis();
        
        // Handle timer register rollover. Restarts task if rollover detected.
        // The problem occurs when the current time rolls over with the target
        // time near rollover; this results in the task stalling until the current
        // timer catches up with the target time (rare, but possible). Since
        // the greatest difference between the targetTime and currentTime should be
        // on the order of the taskInterval, if it is much greater (e.g. 100X) then
        // that indicates a rollover has occurred.
        if (currentTime < targetTime && (targetTime - currentTime) > rolloverThreshold) {
            targetTime = currentTime + taskInterval;
        }
        
        // Handle task timing. Task always runs first time through.
        if (!startedFlag) {
            startedFlag = true;
            targetTime = currentTime + taskInterval;
        }
        else {
            if (currentTime < targetTime) {
                return;
            }
            targetTime += taskInterval;
        }

        // Process voltage readings
        readings[MONITOR_VOLTAGE_POS] = nearest10(ina260Pos.readBusVoltageInt16());
        readings[MONITOR_VOLTAGE_NEG] = -nearest10(ina260Neg.readBusVoltageInt16());
    
        // Process current readings (current always treated as positive)
        readings[MONITOR_CURRENT_POS] = ina260Pos.readCurrentInt16() + 3;  /// @todo Use calibration data to correct positive current reading
        readings[MONITOR_CURRENT_NEG] = ina260Neg.readCurrentInt16() + 2;  /// @todo Use calibration data to correct negative current reading

        // Check all voltages and current against specifications (all currents are treated as positive)
        alert[MONITOR_VOLTAGE_POS] = (readings[MONITOR_VOLTAGE_POS] > LIMIT_MAX_VOLTAGE);
        alert[MONITOR_VOLTAGE_NEG] = (readings[MONITOR_VOLTAGE_NEG] < -LIMIT_MAX_VOLTAGE);
        alert[MONITOR_CURRENT_POS] = (readings[MONITOR_CURRENT_POS] > LIMIT_MAX_CURRENT);
        alert[MONITOR_CURRENT_NEG] = (readings[MONITOR_CURRENT_NEG] > LIMIT_MAX_CURRENT);

        // Alert on voltage or current out of range. Beep every OVER_RANGE_BEEP_N times
        // the Monitor task runs.
        if (alert[MONITOR_VOLTAGE_POS] || alert[MONITOR_VOLTAGE_NEG] || alert[MONITOR_CURRENT_POS] || alert[MONITOR_CURRENT_NEG]) {
            if (beep_count <= 0) {
                beep_count = OVER_RANGE_BEEP_N;
                BuzzerTask::beep(BEEP_BLIP, 1);
            }
            else {
                beep_count--;
            }
        }
        
        // Display voltage data
        lcd->setCursor(0, 0);
        lcd->print(F("V  "));
        lcd->print(generateVoltageString(readings[MONITOR_VOLTAGE_POS]));
        lcd->print(" ");
        lcd->print(generateVoltageString(readings[MONITOR_VOLTAGE_NEG]));

        // Display current data (note: current is always considered positive to avoid
        // cluttering the display).
        lcd->setCursor(0, 1);
        lcd->print(F("mA  "));
        (void) sprintf(string_buf, "% 5d ", readings[MONITOR_CURRENT_POS]);
        lcd->print(string_buf);
        lcd->print(" ");
        (void) sprintf(string_buf, "% 5d", readings[MONITOR_CURRENT_NEG]);
        lcd->print(string_buf);
    }

    /**
    * @brief Reads and returns the raw, unscaled and uncorrected ADC values for all measurements.
    *
    * Reads and returns the raw, unscaled and uncorrected ADC values for all measurements.
    *
    * This function supports the Calibrate task and is safe to call after setup but
    * when the Monitor task is not being scheduled (i.e. during calibration).
    * Note that the global readings array is used here for raw readings, so this must not
    * be called when the monitor task is being scheduled.
    *
    * @param[out] readings[]   Returns results in global array
    */
    void getRawValues(void) {
        // Process voltage readings
        readings[MONITOR_VOLTAGE_POS] = ina260Pos.readBusVoltageRaw();
        readings[MONITOR_VOLTAGE_NEG] = ina260Neg.readBusVoltageRaw();
    
        // Process current readings (current always treated as positive)
        readings[MONITOR_CURRENT_POS] = ina260Pos.readCurrentRaw();
        readings[MONITOR_CURRENT_NEG] = ina260Neg.readCurrentRaw();
    }

    /**
    * @brief Round integer to nearest multiple of 10.
    *
    * @return Rounded integer
    */
    int16_t nearest10(int16_t value) {
        int16_t a = (value / 10) * 10;
        if ((value - a) >= 5) {
            return a + 10;
        }
        return a;
    }

    /**
    * @brief Generates a voltage string from the provided integer value.
    *
    * Generate voltage string from provided value. Takes value in millivolts and presents as decimal
    * volts with precision to hundredths of a volt.
    *
    * @param value   Integer value of voltage in millivolts
    *
    * @return String representing decimal volts to thousandths of a volt
    */
    char* generateVoltageString(int16_t value) {
        // Generate the string from integer values
        (void) sprintf(string_buf, "%+ 5d", value / 10);
        // Set seventh character to nul to terminate string
        string_buf[6] = 0;
        // Move rightmost 2 digits to the right
        string_buf[5] = string_buf[4];
        string_buf[4] = string_buf[3];
        // Add decimal point
        string_buf[3] = '.';
        return string_buf;
    }

}