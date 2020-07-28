# AVR Libraries and Applications

This is the codebase for application development on the ATmega328P AVR microcontroller.

2KB RAM / 32KB progmem, no STL available, so code is generally tight, no dynamic allocations.\
Has GCC C++17 compiler support though so at least there's that.

Contains several functional areas for interfacing with the main subsystems of the AVR.
This includes:
- Interrupt management
- HW Timer management
- Serial interface
- I2C
- Pin control (IO), low power, sleep modes, clock frequencies, watchdogs, etc

In addition to the core libraries, application libraries exist as well, such as:
- Event management (timers, userspace ISR, idle time processing)
- Peripheral interface objects (radio modules, sensors, motors)
- Signal synthesis (music, RF modulation)
- RF Protocols (APRS, WSPR)
- Utility functions (containers, string parsers/formatters, etc)

Built on top of the above is a number of applications (prefixed with "App").
