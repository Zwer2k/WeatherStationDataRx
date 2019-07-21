

#ifndef WeatherStationDataRx_h
#define WeatherStationDataRx_h
#include <Arduino.h>

// Define where debug output will be printed.
#define DEBUG_PRINTER Serial

// Setup debug printing macros.
#ifdef WSDR_DEBUG
  #define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
  #define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
  #define DEBUG_PRINT(...) {}
  #define DEBUG_PRINTLN(...) {}
#endif

class WeatherStationDataRx {
public:
    WeatherStationDataRx(uint8_t dataPin);
    void begin();

    char readData();
    
    float readTemperature(bool inF = false);
    uint8_t readHumidity();
    float readWindSpeed(bool inKMH = false);
    uint16_t readWindDirection();
    float readWindGust(bool inKMH = false); 
    float readRainVolume();

    float convertCtoF(float);
    float convertFtoC(float);

private:
    uint8_t dataPin;
    uint16_t temperature, humidity, windSpeed, windDirection, windGust, rainVolume; // Variablen zum speichern der Daten
    byte batteryState = 0; // der Batterie-Status von beiden Sensoren (Bit 0 = Windsensor und Bit 1 = Regensensor)

    bool calculateChecksume(byte startValue, bool add);
};

#endif