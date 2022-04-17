/**************************************************************************************

  This is example for read weather data from Ventus W174/W132 (tested), Auriol H13726, Hama EWS 1500, Meteoscan W155/W160

  Initial Date: 21-July-2019
  Latest Rev: 5-Oct-2020

  Hardware connections for Arduino:
  VDD to 3.3V DC (RXB6) or 5V DC (MX-RM-5V)
  D3 from data pin
  GND to common ground

  Hardware connections for ESP8266 (Tested on Wemos D1 Mini) / ESP32 (ESP-WROOM-32):
  VDD to 3.3V DC (RXB6) or 5V DC (MX-RM-5V)
  D5 from data pin
  GND to common ground


  MIT License

  Authors:
  - Zwer2k - https://github.com/Zwer2k
  - Simone Fardella - https://github.com/simonefardella


**************************************************************************************/

#include "WeatherStationDataRx.h"

#ifdef ESP8266
#define DATA_PIN D5
#else
#define DATA_PIN 2
#endif

WeatherStationDataRx wsdr(DATA_PIN, false, ARMUseAsConfirmation, false);

void PairedDeviceAdded(byte newID)
{
#if defined(ESP8266) || defined(ESP32)
    Serial.printf("New device paired %d\r\n", newID);
#else
    Serial.print("New device paired ");
    Serial.println(newID, DEC);
#endif

    wsdr.pair(PairedDeviceAdded);

    // If you already have a sensor ID for your device, you can set it as follows.
    // byte myDeviceIDs[] = {34, 63};
    // wsdr.pair(myDeviceIDs, sizeof(myDeviceIDs));
}

void setup()
{
    Serial.begin(115200);
    delay(5000);
    Serial.println("WeatherStationDataRx Test");

    wsdr.begin();
    wsdr.pair(PairedDeviceAdded);
}

void loop()
{
    byte newDataState = wsdr.readData(true);
    if (newDataState > 0) {
        Serial.print("ensorID ");
        Serial.println(wsdr.sensorID());
        
        if (wsdr.dataHas(newDataState, NDTemperature)) {
            Serial.print("Battery: ");
            Serial.println(bitRead(wsdr.batteryStatus(), 0) == 0 ? "OK" : "Low");
        }

        if (wsdr.dataHas(newDataState, NDTemperature)) {
            Serial.print("Temperature: ");
            Serial.print(wsdr.readTemperature());
            Serial.println("*C");
        }
        
        if (wsdr.dataHas(newDataState, NDHumidity)) {
            Serial.print("Humidity: ");
            Serial.print(wsdr.readHumidity());
            Serial.println("%");
        }

        if (wsdr.dataHas(newDataState, NDWindSpeed)) {
            Serial.print("Wind speed: ");
            Serial.print(wsdr.readWindSpeed());
            Serial.println("m/s");
        }
        
        if (wsdr.dataHas(newDataState, NDWindDirection)) {
            Serial.print("Wind direction: ");
            Serial.print(wsdr.readWindDirection());
            Serial.println("°");
        }
        
        if (wsdr.dataHas(newDataState, NDWindGust)) {
            Serial.print("Wind gust: ");
            Serial.print(wsdr.readWindGust());
            Serial.println("m/s");
        }
    
        if (wsdr.dataHas(newDataState, NDRainVolume)) {
            Serial.print("Rain volume: ");
            Serial.print(wsdr.readRainVolume());
            Serial.println("mm");
        }

        if (wsdr.dataHas(newDataState, NDError)) {
            Serial.println("Error: pairing required");
        }

        Serial.println("------------");
    }
}
