/**
 * @file BuzzerTask.cpp
 * @copyright
 * Copyright 2024 Gregory Aicklen.
 * Licensed under the MIT License
 *
 * @brief Implements a BuzzerTask that manages buzzer activation, duration, and repetition.
 *
 * Implements a BuzzerTask that manages an active buzzer attached to an Arduino digital pin.

 * To use the Buzzer task:
 *      - BuzzerTask::setup() - setup the task
 *      - BuzzerTask::update() - run the buzzer process; call once each time through scheduler
 *      - BuzzerTask::beep() - call each time a sequence of beeps are desired
 *      - BuzzerTask::mute() - call to disable buzzer sounds, but otherwise don't change buzzer function
 *      - BuzzerTask::unmute() - call to restore buzzer sounds, but otherwise don't change buzzer function
 *      - BuzzerTask::toggleMute() - call to turn toggle the state of buzzer muting
 *
 * Intended to be run by a simple, non-preemptive round robin scheduler, where each task
 * is responsible for relinquishing control; this code is *not threadsafe* and must not be
 * interrupted.
 */

// Include our own header file
#include "BuzzerTask.h"

namespace BuzzerTask {

    bool muteMode = false;

    uint8_t buzzerPin;
    uint8_t buzzerOn;
    uint8_t buzzerOff;

    struct {
        unsigned duration = 0;
        unsigned count = 0;
        bool doingBeepSpacing = false;
    } beepData;

    // Task timing (all in milliseconds)
    unsigned long rolloverThreshold;  // Should be 10*taskInterval
    unsigned long currentTime;
    unsigned long targetTime = 0;

    /**
    * @brief Configures the Buzzer task to the Arduino hardware in use.
    *
    * @param pin       The digital pin that the buzzer is connected to
    * @param on_value  The value (HIGH or LOW) that turns on the buzzer
    * @param off_value The value (HIGH or LOW) that turns off the buzzer
    */
    void setup(uint8_t pin,
                        uint8_t on_value,
                        uint8_t off_value) {
        // Should be 100 * longest beep duration
        rolloverThreshold = (unsigned long)100 * (unsigned long)BEEP_LONG;

        buzzerPin = pin;
        buzzerOn = on_value;
        buzzerOff = off_value;
    }

    /**
    * @brief Sounds the buzzer for a selectable time one or more times.
    *
    * @param duration  The time in milliseconds to sound the buzzer
    * @param count     The number of times to sound the buzzer
    */
    void beep(unsigned duration, unsigned count) {
        if (count == 0) {
            return;
        }
        if (!muteMode) {
            currentTime = millis();
            digitalWrite(buzzerPin, buzzerOn);
            beepData.duration = duration;
            beepData.count = count;
            beepData.doingBeepSpacing = false;
            targetTime = currentTime + duration;
            beepData.count--;
        }
    }

    /**
    * @brief Called to mute the buzzer.
    *
    */
    void mute(void) {
        beep(BEEP_BLIP, 2);
        muteMode = true;
    }

    /**
    * @brief Called to unmute the buzzer.
    *
    */
    void unmute(void) {
        muteMode = false;
        beep(BEEP_BLIP, 3);
    }

    /**
    * @brief Called to toggle the mute state of the buzzer.
    *
    */
    void toggleMute(void) {
        if (muteMode) {
            unmute();
        }
        else {
            mute();
        }
    }

    /**
    * @brief Called each time through the scheduling loop to manage the buzzer.
    *
    * Implements the BuzzerTask. Runs whenever the scheduler transfers control to the
    * BuzzerTask and determines if it is time to run; if not time to run, immediately
    * relinquishes control back to the scheduler.
    */
    void update(void) {
        currentTime = millis();
        
        // Handle timer register rollover.
        // The problem occurs when the current time rolls over with the target
        // time near rollover; this results in the task stalling until the current
        // time catches up with the target time (rare, but possible). Since
        // the greatest difference between the targetTime and currentTime should be
        // on the order of the longest buzzer time, if it is much greater (e.g. 100X) then
        // that indicates a rollover has occurred.
        if ((currentTime >= targetTime) || (targetTime - currentTime) > rolloverThreshold) {
            if (beepData.count == 0) {
                digitalWrite(buzzerPin, buzzerOff);
                return;
            }
            // Note: to get here beepData.count must be > 0.
            if (!beepData.doingBeepSpacing) {
                digitalWrite(buzzerPin, buzzerOff);
                beepData.doingBeepSpacing = true;
                targetTime += BEEP_SPACING;
            }
            else {
                digitalWrite(buzzerPin, buzzerOn);
                beepData.doingBeepSpacing = false;
                targetTime += beepData.duration;
                beepData.count--;
            }
        }
    }
}