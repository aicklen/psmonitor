
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)(https://github.com/aicklen/psmonitor/blob/master/LICENSE))
[![GitHub release](https://img.shields.io/github/v/release/aicklen/psmonitor)(https://github.com/aicklen/psmonitor/releases)

# PSMonitor

Arduino sketch implementing a voltage and current monitor for dual power supply with symmetric outputs.

## Description

Working. Tested on prototype hardware.

This sketch implements the firmware for a voltage and current monitor that can be retrofitted
to an existing symmetric, dual output power supply (i.e. a supply with positive and negative
outputs of the same magnitude). The firmware is for a specific hardware design.

Normally, the sketch will take current and voltage measurements for both positive
and negative supplies, correct for measurement errors, and display on the LCD.
If turned on while holding down the mute button, the firmware will go into
calibration mode where the user will be directed to set specific voltages and currents
at the power supply outputs using a trusted DMM; corrections for measurement errors
will be computed and stored to EEPROM for subsequent use in normal mode.

This firmware provides an audible warning if any output voltage or current
exceeds the maximum specifications for the supply. The warning can be muted
by pressing the mute button once, and unmuted by pressing the mute button again.


### Hardware

TBD

### Notes on Implementation

TBD

## Future

### Must

- Implement calibration function inside the calibration task
- Move all configuration values into a single include file

### Should

- Implement a method to cancel calibration mode with no changes to calibration data

### Could

- Improve displayed calibration instructions
