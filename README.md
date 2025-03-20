# Code for 6Sens Project : final code

## Overview
Here is the final code for the 6Sens (6Sens: inSitu Seismic System for Slope Stability Sensing) project. It makes it possible to measure seismic measurements and alert in the event of an alert. The data is stored in memory when an event appears and, when it stops, all samples are sent to the Lorawan networks servers then to a 4G network. All servers are implemeted in a Raspberry Pi3 model B and this gateway is powered by solar panel. In addition, the battery level and temperature/pressure/humidity of the node are taken every 30 minutes and sent to the network once a day.

## Building and Running
The following commands clean build folder, build and flash the sample:

**Command to use**

west build -t pristine

west build -p always -b mdbt50q_lora_dev applications/nrf52840_rtos_6sens

west flash --runner pyocd