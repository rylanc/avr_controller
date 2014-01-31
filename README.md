A small service that triggers settings changes on Denon receivers. I currently use it to turn on and set the current input/volume/etc. when [shairport](https://github.com/abrasive/shairport) begins playing. In the future, I hope to expand it to support other receivers (through the config.xml).

To trigger the start commands, run start.py.
When stop.py is run, the service will wait 90 seconds (this is configurable) before sending the stop commands. If start.py is run before the timer expires, the stop timer is cancelled.
