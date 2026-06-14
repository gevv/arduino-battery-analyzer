# Arduino Battery Analyzer

This project is an Arduino-based battery analyzer designed for measuring battery capacity and estimating internal resistance. It can be used not only for single-cell 18650 Li-ion batteries, but also for other rechargeable batteries and battery packs up to approximately 20 V, depending on the voltage divider, load resistor, MOSFET, cooling, and calibration settings.

Related article/project page:
https://320volt.com/arduino-pil-analiz-cihazi

## Features

* Battery capacity test
* Internal resistance estimation
* Support for single-cell 18650 Li-ion batteries
* Can be adapted for other batteries or battery packs up to approximately 20 V
* Different load current options depending on the load resistor and MOSFET
* Measurement monitoring via LCD screen
* Fan-assisted load resistor cooling

## Hardware

* Arduino Nano / ATmega328P
* Battery holder or external battery connection terminals
* Load resistors
* MOSFET driver circuit
* LCD screen
* Voltage measurement divider circuit
* Current measurement circuit
* Cooling fan for load resistors and MOSFET

## Important Note

This project is for educational and testing purposes only. When working with Li-ion batteries, lead-acid batteries, NiMH packs, or other rechargeable batteries, caution must be exercised against short circuits, overcurrent, reverse polarity, overheating, and incorrect voltage divider calibration.

For battery voltages higher than a single Li-ion cell, the voltage divider, load resistor power rating, MOSFET voltage/current rating, heat dissipation, and firmware calibration values must be checked and adjusted before use.

## Credits

Original project:
https://tefatronix.g6.cz/display.php?page=batmeter

Original author:
F. Štefanec

This Arduino Nano version is based on / adapted from the original rechargeable battery analyzer project.
