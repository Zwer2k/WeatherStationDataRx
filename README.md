# WeatherStationDataRx
Arduino library for read weather data from Ventus W174/W132 (tested), Auriol H13726, Hama EWS 1500, Meteoscan W155/W160

The transmission protocol of the weather station is described here: 
http://www.tfd.hu/tfdhu/files/wsprotocol/auriol_protocol_v20.pdf

The communication can be done via receiver module RXB6/MX-RM-5V or directly by modification at the transmitter (e.g. with Ventus W132). 

Tested on Arduino Duemilanove, ESP8266 (Tested on Wemos D1 Mini) and ESP32 (ESP-WROOM-32
### Communication via the receiver module
![Connecting RXB6](doc/RXB6_connect.png)

### Communication by modification at the transmitter Ventus W132
![Connecting RXB6](doc/W132_connect.png)

![Connecting RXB6](doc/W132_board.jpg)


## Changelog

### v0.5.0
- Packet confirmation by duplicates of packets. As a result, there should no longer be any erroneous readings.

## License

The MIT License (MIT)

Copyright (c) 2020 Zwer2k - Fardella Simone - Martin Korbel
