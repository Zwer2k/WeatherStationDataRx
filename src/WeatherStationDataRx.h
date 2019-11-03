

#ifndef WeatherStationDataRx_h
#define WeatherStationDataRx_h
#include <Arduino.h>

//#define WSDR_DEBUG
// Define where debug output will be printed.
#define DEBUG_PRINTER Serial

// Setup debug printing macros.
#ifdef WSDR_DEBUG
#define DEBUG_PRINT(...)                  \
    {                                     \
        DEBUG_PRINTER.print(__VA_ARGS__); \
    }
#define DEBUG_PRINTLN(...)                  \
    {                                       \
        DEBUG_PRINTER.println(__VA_ARGS__); \
    }
#define DEBUG_PRINTF(...)                  \
    {                                      \
        DEBUG_PRINTER.printf(__VA_ARGS__); \
    }
#else
#define DEBUG_PRINT(...) \
    {                    \
    }
#define DEBUG_PRINTLN(...) \
    {                      \
    }
#define DEBUG_PRINTF(...) \
    {                     \
    }
#endif

class WeatherStationDataRx
{
public:
    WeatherStationDataRx(uint8_t dataPin, bool pairingRequired = false);
    ~WeatherStationDataRx();

    void begin();
    void pair(byte pairedDevices[] = NULL, void (*pairedDeviceAdded)(byte newID) = NULL);

    char readData();

    float readTemperature(bool inF = false);
    uint8_t readHumidity();
    float readWindSpeed(bool inKMH = false);
    uint16_t readWindDirection();
    float readWindGust(bool inKMH = false);
    float readRainVolume();
    bool readButtonState();

    float convertCtoF(float);
    float convertFtoC(float);

private:
    static WeatherStationDataRx *__instance[4];
    uint8_t dataPin;
    bool pairingRequired;
    byte *pairedDevices = NULL;
    byte pairedDevicesCount = 0;
    unsigned long pairingEndMillis = 0;
    void (*pairedDeviceAdded)(byte newID) = NULL;
#ifdef WSDR_DEBUG
    bool pairingRequeredMessageSent = false;
#endif
    bool buttonState;
    uint16_t temperature, humidity, windSpeed, windDirection, windGust, rainVolume; // Variablen zum speichern der Daten
    byte batteryState = 0;                                                          // der Batterie-Status von beiden Sensoren (Bit 0 = Windsensor und Bit 1 = Regensensor)

    bool calculateChecksume(byte startValue, bool add);
    bool isPaired(byte randomID);
    bool pairingDevice(byte randomID, byte xBits, byte subID = 0);

protected:
    static void _ISR();
    void rx433Handler();
};

#endif
