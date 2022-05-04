

#ifndef WeatherStationDataRx_h
#define WeatherStationDataRx_h
#include <Arduino.h>

//#define WSDR_DEBUG
// Define where debug output will be printed.
#define DEBUG_PRINTER Serial

// Time within which similar messages are ignored
#define IGNORE_REPEATED_MESSAGES_TIME 3000

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

enum NewDataType {
    NDBattState     = B00000001,
    NDTemperature   = B00000010,
    NDHumidity      = B00000100,
    NDWindSpeed     = B00001000,
    NDWindDirection = B00010000,
    NDWindGust      = B00100000,
    NDRainVolume    = B01000000,
    NDError         = B10000000,
};

enum ActionOnRepeatedMessage {
    ARMUseAsConfirmation2x,  // Duplicate packages are used as confirmation package. 2 confirmation packages are expected. (safe method)
    ARMUseAsConfirmation,    // Duplicate packages are used as confirmation package
    ARMIgnore,               // Duplicate packets are ignored 
    ARMPass                  // Duplicate packages are passed on  
};

class WeatherStationDataRx
{
public:
    WeatherStationDataRx(uint8_t dataPin, bool pairingRequired = false, ActionOnRepeatedMessage actionOnRepeatedMessage = ARMUseAsConfirmation2x, bool keepNewDataState = false);
    ~WeatherStationDataRx();

    void begin();
    void end();
    void pair(void (*pairedDeviceAdded)(byte newID) = NULL);
    void pair(byte pairedDevices[], byte pairedDevicesCount, void (*pairedDeviceAdded)(byte newID) = NULL);

    byte readData(bool newFormat = false);                      // read data from buffer and retun the state                                                                
                                                                // in the new format one bit defines a measurement result
                                                                //  you can check if a measurement result is available with the function dataHas()
                                                                // old state format has an char () 
                                                                //  T = temperature+ humidity, 
                                                                //  S = wind speed,
                                                                //  G = wind direction and wind gust
                                                                //  R = rain volume 
                                                                 
    bool dataHas(byte newDataState, NewDataType check);

    float readTemperature(bool inF = false);
    uint8_t readHumidity();
    float readWindSpeed(bool inKMH = false);
    // 0 - N, 45 - NE, 90 - E, 135 - SE, 180 - S, 225 - SW, 270 - W, 315 - NW
    uint16_t readWindDirection();
    float readWindGust(bool inKMH = false);
    float readRainVolume();
    bool readButtonState();
    byte batteryStatus(); // 0 - OK, 1 - wind sensor, 2 - rain sensor, 3 - both
    byte sensorID();

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
    byte newData = 0;
    int16_t temperature;
    uint16_t humidity, windSpeed, windDirection, windGust, rainVolume; // Variablen zum speichern der Daten
    byte batteryState = 0;                                                          // der Batterie-Status von beiden Sensoren (Bit 0 = Windsensor und Bit 1 = Regensensor)
    byte randomID = 0;                                                              // At power up (when the batteries are inserted) the sensor selects a random number.
    ActionOnRepeatedMessage actionOnRepeatedMessage;
    bool keepNewDataState;
    unsigned long lastDataTime = 0;
    volatile bool bufferReadLock = false;
    volatile bool bufferWriteLock = false;

    bool calculateChecksume(unsigned long long data, byte startValue, bool add);
    bool isPaired(byte randomID);
    bool pairingDevice(byte randomID, byte xBits, byte subID = 0);

protected:
    static void _ISR();
    void rx433Handler();
};

#endif
