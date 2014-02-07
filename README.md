avr_controller
=================
A small service that triggers settings changes on Denon AVR receivers over serial port (RS-232). I currently use it to turn on my receiver and set the current input/volume/etc when [shairport](https://github.com/abrasive/shairport) begins playing on my Raspberry Pi. In the future, I hope to expand it to support other receivers (through [the config file](config.xml)).

Prerequisites
-------------
- [boost](http://www.boost.org)
- A C++11 compatible compiler (tested with GCC 4.7)

Getting Started
-------------
To build and start the process:
```bash
make
./avr_controller
```
To trigger a state, run the avr_command Python script:
```bash
./avr_command PLAY # Triggers the play events
./avr_command STOP # Triggers the stop events
```
The service will wait 90 seconds (this is configurable) before triggering the stop events. If a play command is received before the timer expires, the stop timer is cancelled.
