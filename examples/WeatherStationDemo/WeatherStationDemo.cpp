/**************************************************************************************

This is example for read weather data from Venus W174/W132 (tested), Auriol H13726, Hama EWS 1500, Meteoscan W155/W160

Initial Date: 21-July-2019

Hardware connections for Arduino / ESP8266:
VDD to 3.3V DC
D3 data pin from  
GND to common ground

MIT License

**************************************************************************************/

#include "WeatherStationDataRx.h"

#ifdef ESP8266
#define DATA_PIN D5
#else
#define DATA_PIN 3
#endif

WeatherStationDataRx wsdr(DATA_PIN, true);

void PairedDeviceAdded(byte newID) {
    Serial.printf("New device paired %d\r\n", newID);
    wsdr.pair(NULL, PairedDeviceAdded);
}

void setup()
{
	Serial.begin(115200);
    delay(5000);
	Serial.println("WeatherStationDataRx Test");

	wsdr.begin();
    wsdr.pair(NULL, PairedDeviceAdded);
}

void loop()
{
    char newData = wsdr.readData();
	switch(newData) {
        case 'T':
            Serial.print("Temperature: ");
            Serial.print(wsdr.readTemperature());
            Serial.print("°");
            Serial.println("C");
            Serial.print("Humidity: ");
            Serial.print(wsdr.readHumidity());
            Serial.println("%");
            break;
            
        case 'S':
            Serial.print("Wind speed: ");
            Serial.print(wsdr.readWindSpeed());
            Serial.println("m/s");
            break;

        case 'G':
            Serial.print("Wind direction: ");
            Serial.print(wsdr.readWindDirection());
            Serial.println("°");
            Serial.print("Wind gust: ");
            Serial.print(wsdr.readWindGust());
            Serial.println("m/s");
            break;

        case 'R':
            Serial.print("Rain volume: ");
            Serial.print(wsdr.readRainVolume());
            Serial.println("mm");
            break;

        default:
            break;
    }
}
