/**
 * @file psmonitor.ino
 * @copyright
 * Copyright 2024 Gregory Aicklen.
 * Licensed under the MIT License
 *
 * @brief Monitor and display for dual adjustable power supplies with symmetric positive and negative outputs.
 *
 * This sketch implements the firmware for a voltage and current monitor that can be retrofitted
 * to an existing symmetric, dual output power supply (i.e. a supply with positive and negative
 * outputs of the same magnitude). The firmware is for a custom hardware design.
 *
 * Normally, the sketch will take current and voltage measurements for both positive
 * and negative supplies, correct for measurement errors, and display on the LCD.
 * If turned on while holding down the mute button, the firmware will go into
 * calibration mode where the user will be directed to set specific voltages and currents
 * at the power supply outputs using a trusted DMM; corrections for measurement errors
 * will be computed and stored to EEPROM for subsequent use in normal mode.
 *
 * This firmware provides an audible warning if any output voltage or current
 * exceeds the maximum specifications for the supply. The warning can be muted
 * by pressing the mute button once, and unmuted by pressing the mute button again.
 */

// Include standard headers as needed
#include <Arduino.h>
#include <stdio.h>

// Include third-party library header files
#include "Adafruit_INA260.h"
#include <LiquidCrystal.h>

// Include task header files
#include "globals.h"
#include "BuzzerTask.h"
#include "CalibrateTask.h"
#include "MonitorTask.h"

// Project specific headers
#include "Calibration.h"

// Create an LCD object.
// Initialize the library by mapping any LCD interface pins to the
// matching arduino pin number.
#define RS 12
#define EN 11
#define D2 2
#define D3 3
#define D4 4
#define D5 5
LiquidCrystal lcd(RS, EN, D2, D3, D4, D5);

// Digital I/O lines used for other than the LCD
#define BUZZER_PIN 6
#define BUTTON_PIN 7

// Various useful state variables
byte previous_button_state = HIGH;
#define MODE_NORMAL 0
#define MODE_CALIBRATE 1
#define MODE_TERMINATE 2
byte currentMode = MODE_NORMAL;

// Monitor task setup
#define MONITOR_INTERVAL 200  // Time between runs; in milliseconds
#define MONITOR_POS_ADDR 0x40 // I2C address of positive voltage/current sensor
#define MONITOR_NEG_ADDR 0x41 // I2C address of negative voltage/current sensor

// Buffer for string manipulation; global
char string_buf[17];

/**
* @brief Configures the power supply monitor; runs once on powerup or reset.
*
*/
void setup() {
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(BUTTON_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    // set up the LCD's number of columns and rows and clear the screen:
    lcd.begin(16, 2);
    lcd.clear();

    // Setup the buzzer task
    BuzzerTask::setup(BUZZER_PIN, HIGH, LOW);

    // Setup the monitor task
    MonitorTask::setup(MONITOR_INTERVAL,
                       MONITOR_POS_ADDR,
                       MONITOR_NEG_ADDR, &lcd);

    // Can monitor task communicate with sensors?
    if (!MonitorTask::communicationOK()) {
        lcd.setCursor(5, 0);
		    lcd.print(F("Fault!"));
        lcd.setCursor(0, 1);
        lcd.print(F("Sensor Comm Bad?"));
        BuzzerTask::beep(BEEP_LONG, 3);
        currentMode = MODE_TERMINATE;
    }
    else {
      
        // Initialize monitoring hardware
        MonitorTask::setAveragingCount(INA260_COUNT_16);
        MonitorTask::setConversionTime(INA260_TIME_2_116_ms);

        // Setup the Calibrate task
        CalibrateTask::setup(&lcd);

        // Read existing calibration data, if any
        Calibration::recall();

        // Check the mute/calibrate button; if low at powerup, then set calibrate mode
        if (digitalRead(BUTTON_PIN) == LOW) {
            previous_button_state = LOW;
            BuzzerTask::beep(BEEP_MEDIUM, 2);
            currentMode = MODE_CALIBRATE;
        }
        // If mute/calibrate button not being held down, just beep
        else {
            BuzzerTask::beep(BEEP_SHORT, 1);
        }
    }
}


/**
* @brief Runs continuously after setup; implements a simple non-preempting round-robin scheduler.
*
*/
void loop() {
    // Check mute/calibrate button; toggle mute mode if button state has
    // transitioned from from HIGH to LOW since last check ("edge" triggered)
    if (digitalRead(BUTTON_PIN) == LOW) {
        if (previous_button_state == HIGH) {
            previous_button_state = LOW;
            switch(currentMode) {
            case MODE_CALIBRATE:
                BuzzerTask::beep(BEEP_BLIP, 1);
                CalibrateTask::buttonPress(); // Inform the calibrate task that button was pushed
                break;
            case MODE_NORMAL:
            default:
                BuzzerTask::toggleMute();
                break;
            }
        }
    }
    else {
        previous_button_state = HIGH;
    }

    // Process any requested buzzer soundings
    BuzzerTask::update();

    // Dispatch to monitor or calibrate task, depending on current operational mode.
    switch(currentMode) {
    case MODE_TERMINATE:
        break;
    case MODE_CALIBRATE:
        if (CalibrateTask::finished()) {
            currentMode = MODE_NORMAL;
            BuzzerTask::beep(BEEP_BLIP, 3); // Let the user know calbration is done
            break;
        }
        CalibrateTask::update();
        Calibration::recall();
        break;
    case MODE_NORMAL:
    default:
        // Read and display voltage and current readings.
        // This takes on the order of 15ms elapsed time.
        MonitorTask::update();
        break;
    }
 }
