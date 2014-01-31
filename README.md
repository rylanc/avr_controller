A small service that triggers settings changes on Denon receivers. I currently use it to turn on my receiver and set the current input/volume/etc when [shairport](https://github.com/abrasive/shairport) begins playing on my Raspberry Pi. In the future, I hope to expand it to support other receivers (through [the config file](config.xml)).

The start.py script triggers the start commands.  
The stop.py script triggerts the stop timer. The service will wait 90 seconds (this is configurable) before sending the stop commands. If start.py is run before the timer expires, the stop timer is cancelled.
