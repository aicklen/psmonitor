
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](https://github.com/aicklen/psmonitor/blob/master/LICENSE)
[![GitHub Release](https://img.shields.io/github/v/release/aicklen/psmonitor?include_prereleases)](https://github.com/aicklen/psmonitor/releases)
[![GitHub Issues or Pull Requests](https://img.shields.io/github/issues/aicklen/psmonitor)](https://github.com/aicklen/psmonitor/issues)
[![Documentation](https://github.com/adafruit/ci-arduino/blob/master/assets/doxygen_badge.svg)](https://aicklen.github.io/psmonitor/html/index.html)

# PSMonitor

Hardware design and Arduino sketch implementing a voltage and current monitor for dual power supply with symmetric outputs.

## Description

Status: working. Tested on prototype hardware.

This project provides a custom hardware design and firmware for a voltage and current monitor that can be retrofitted
to an existing symmetric, dual output power supply (i.e. a supply with positive and negative
outputs of the same magnitude) with outputs up to +30/-30 volts.

### Hardware

This sketch is designed to run on specific hardware, shown below. The hardware,
designed using components already in my stock, was intended to be retrofit to an
Elenco XP-720 power supply. The design would work well for many other power
supplies, however, so it's published here.

#### Schematic

![Schematic for Power Supply Monitor](https://github.com/aicklen/psmonitor/blob/master/schematic/psmonitor.png?raw=true)

#### Discussion

The brains of the PSMonitor is an Arduino Nano v3 (A1), which is based on an 8-bit ATmega328 processor with 2KB RAM,
32KB Flash, and 1KB EEPROM. At 16MHz this processor is more than snappy enough for the
application, but the small available RAM means careful attention to memory use is
called for.

Voltage and current monitoring are provided by a pair of INA260 devices from Texas Instruments (U1 and U2).
These ICs, conveniently communicating through I2C, can handle bus voltages from 0V to 36V and
continuous current up to 15A. There is very little voltage drop across the 2mOhm current sense resistor,
and the specifications call for gain error of no more than 0.15% and current measurement offset of
no more than 5mA. These devices are already accurate enough for non-critical applications, but
the firmware provides for calibration to reduce these errors if needed.

Since the INA260 can only measure positive bus voltages, the sensor for the negative supply is
configured so that the "ground" is the negative supply output while the bus voltage is connected
to system ground. The negative side measurement is between system ground and
the negative side floating "ground", which is positive at the magnitude of the negative output.

The INA260 devices operate From a 2.7V to 5.5V supply, which is handy in the implementation of
monitoring for the negative supply. An LM317 is used to generate an approximate 3.3V supply for the
negative side INA260 using the difference between the 5V system supply and the lowest setting
of the negative supply (-1.25V); this value is well within the INA260 specifications while
keeping the input to the LM317 above the dropout voltage.

Additional elements:

- An ISO1540 (U3) is used as an isolator and level shifter for the I2C connection between the negative side sensor
and the Arduino Nano.

- A debounced momentary contact switch is provided (used by firmware to mute audible alerts or enter into
calibration mode).

- The circuit is designed to interface to a standard 1602A LCD display through the parallel interface.
RV1 is used to set the contrast on the display, while R1 sets the display backlight brightness.

- The dashed boxes around the INA260s (U1 and U2) indicate that Adafruit INA260 breakout boards were used,
and the enclosed components are located on those boards.

- Buzzer BZ1 is a small active buzzer driven by the Arduino Nano at 5V and less than 40mA.

- Diodes (D1 and D2) are provided at the sensor inputs to prevent reverse voltages that might exceed specs.

### Firmware

The firmware is supplied in C++ source as a standard Arduino sketch.
Autogenerated documentation for the firmware can be found in the docs directory:
[Doxygen](https://aicklen.github.io/psmonitor/html/index.html).

#### Overview

The firmware operates in two modes: Normal and Calibration. Normal mode is the
default condition and is entered into after powerup. If the mute/calibrate button
is held down during powerup until the Arduino Nano has finished booting, then
Calibration mode will be entered. Once calibration is completed, the firmware transitions to
Normal mode and continues in that mode until the next power cycle.

_Note: If Calibration mode is entered inadvertently, cycling power (or resetting the Arduino if the
reset button is accessible) is required to exit Calibration mode without changing any existing calibration data._

Normally, the firmware will take current and voltage measurements for both positive
and negative supplies, correct for measurement errors, and display on the LCD.
If the firmware enters into
Calibration mode, the user will be directed to set specific voltages and currents
at the power supply outputs using a trusted DMM; corrections for measurement errors
will be computed and stored to EEPROM for subsequent use in normal mode.

This firmware provides an audible warning while any output voltage or current
exceeds the maximum specifications for the supply. This feature is included
because the target power supply can be adjusted to voltages above the specifications,
and because the target is not current limited. The warning can be muted
by pressing the mute button once, and unmuted by pressing the mute button again.

#### Organization

The PSMonitor sketch is organized into three tasks: BuzzerTask, MonitorTask, and CalibrationTask.
Tasks are implemented in separate namespaces (the C++ equivalent of static classes) and are intended
to be called by a non-preempting round-robin scheduler; the tasks assume that they will not be interrupted
and decide when to return to the scheduler.

#### setup()

The sketch setup() function sets the Arduino Nano pin modes, starts up the LCD display, and
initializes the tasks through each tasks setup() function. Once the MonitorTask has been setup,
the sketch setup() checks that communication with the voltage/current sensors has been established
and then sets averaging and sample time parameters for the sensors. Finally, the setup() function
checks to see if the mute/calibrate button is being held down: if so, Calibrate mode is set and an
audio signal (two medium beeps) is given; otherwise, Normal mode is set and a short beep indicates PSMonitor is ready.

#### loop()

The sketch loop() function implements a simple non-preempting round-robin scheduler that expects
each scheduled task to decide if it is time to run and to return once a stopping point is reached.

1. Checks button state, and determines if the mute/calibrate button has been pressed since the last time
through the loop. If the button has been pressed:
    - If in Normal mode, call the BuzzerTask to toggle muting
    - If in Calibrate mode, call the CalibrateTask to notify that there was a button press
2. Call the BuzzerTask update() function (run the task)
3. Check the current mode:
    - If in Calibrate mode, check CalibrateTask finished() function to see if calibration
is complete and Normal mode should be entered
    - If in Calibrate mode and not finished the calibration procedure, call the CalibrateTask update() function
    - If in Normal mode, call the MonitorTask update() function

## Future

### Must

- Implement calibration function inside the calibration task
- Move all configuration values into a single include file

### Should

- Implement a method to cancel calibration mode with no changes to calibration data
- Improve the documentation

### Could

- Improve displayed calibration instructions
- Include an option to control the 1602A LCD display through I2C
