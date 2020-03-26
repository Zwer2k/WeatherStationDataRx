

#include "WeatherStationDataRx.h"
#include <Arduino.h>

byte dataPin;
volatile unsigned long long rxBuffer; // Variable zum speichern des Datentelegramms (32 Bit)
volatile bool rxOk = false;           // Variable zum anzeigen, das die Daten im Puffer vollstaendig sind

// Helper for ISR call
WeatherStationDataRx *WeatherStationDataRx::__instance[4] = {0};

#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR void WeatherStationDataRx::_ISR()
#else
void WeatherStationDataRx::_ISR()
#endif
{
    for (byte i = 0; i < 4; i++)
        if (__instance[i])
        {
            __instance[i]->rx433Handler();
        }
}

void WeatherStationDataRx::rx433Handler()
{ // Interrupt-Routine
    if (rxOk)
        return; // wenn rxOk noch gesetzt ist, dann Funktion verlassen, damit der Puffer nicht ueberschrieben wird
    static unsigned long rxHigh = 0, rxLow = 0;
    static bool syncBit = 0, dataBit = 0;
    static byte rxCounter = 0;
    bool rxState = digitalRead(dataPin); // Daten-Eingang auslesen
    if (!rxState)
    {                     // wenn Eingang = Low, dann...
        rxLow = micros(); // Mikrosekunden fuer Low merken
    }
    else
    {                      // ansonsten (Eingang = High)
        rxHigh = micros(); // Mikrosekunden fuer High merken
        if (rxHigh - rxLow > 8500)
        {                  // High-Impuls laenger als 8.5 ms dann SyncBit erkannt (9 ms mit 0.5 ms Toleranz)
            syncBit = 1;   // syncBit setzen
            rxBuffer = 0;  // den Puffer loeschen
            rxCounter = 0; // den Counter auf 0 setzen
            return;        // auf den naechsten Interrupt warten (Funktion verlassen)
        }
        if (syncBit)
        { // wenn das SyncBit erkannt wurde, dann High-Impuls auswerten:
            if (rxHigh - rxLow > 1500)
                dataBit = 0; // High-Impuls laenger als 1.5 ms (2 ms mit 0.5 ms Tolenz), dann DataBit = 0
            if (rxHigh - rxLow > 3500)
                dataBit = 1; // High-Impuls laenger als 3.5 ms (4 ms mit 0.5 ms Tolenz), dann DataBit = 1
            if (rxCounter < 36)
            {                                                           // Wenn noch keine 32 Bits uebertragen wurden, dann...
                rxBuffer |= (unsigned long long)dataBit << rxCounter++; // das Datenbit in den Puffer schieben und den Counter erhoehen
            }
            if (rxCounter == 36)
            {                  // wenn das Datentelegramm (32 Bit) vollstaendig uebertragen wurde, dann...
                rxCounter = 0; // den Counter zuruecksetzen
                syncBit = 0;   // syncBit zuruecksetzen
                rxOk = true;   // Ok signalisieren (rxBuffer ist vollstaendig) fuer die Auswertung in Loop
            }
        }
    }
}

WeatherStationDataRx::WeatherStationDataRx(uint8_t dataPin, bool pairingRequired)
{
    for (byte i = 0; i < 4; i++)
        if (__instance[i] == 0)
        {
            __instance[i] = this;
            DEBUG_PRINTLN("::");
            break;
        }

    this->dataPin = dataPin;
    this->pairingRequired = pairingRequired;
}

WeatherStationDataRx::~WeatherStationDataRx()
{
    for (byte i = 0; i < 4; i++)
        if (__instance[i] == this)
        {
            __instance[i] = 0;
        }
}

void WeatherStationDataRx::begin()
{
    // set up the pins!
    DEBUG_PRINTF("Data pin %d\r\n", dataPin);
    Serial.print("Data pin ");
    Serial.println(dataPin);
    pinMode(dataPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(dataPin), _ISR, CHANGE);

    DEBUG_PRINTLN("WeatherStationDataRx begin");
}

void WeatherStationDataRx::pair(byte pairedDevices[], void (*pairedDeviceAdded)(byte newID))
{
    this->pairedDeviceAdded = pairedDeviceAdded;

    if (pairedDevices == NULL)
    {
        DEBUG_PRINTLN("Start pairing for 1 min.");
        pairingEndMillis = millis() + 60 * 1000L;
    }
    else
    {
        int count = sizeof(pairedDevices);
        this->pairedDevices = (byte *)realloc(pairedDevices, count * sizeof(byte));
        memcpy(this->pairedDevices, pairedDevices, count);
    }
}

char WeatherStationDataRx::readData()
{
    if ((pairingEndMillis != 0) && (millis() > pairingEndMillis))
    {
        pairingEndMillis = 0;
        DEBUG_PRINTLN("End pairing, no new devices paired.");
    }

    if ((pairingRequired) && (pairedDevicesCount == 0) && (pairingEndMillis == 0))
    {
#ifdef WSDR_DEBUG
        if (!pairingRequeredMessageSent)
        {
            DEBUG_PRINTLN("Pairing requered.");
            pairingRequeredMessageSent = true;
        }
#endif
        return 'E';
    }

    char newData = 0;

    if (rxOk)
    {                                                       // wenn die Interrupt-Routine rxOk auf true gesetzt hat, dann ist der Puffer mit den Daten gefuellt
        randomID = (unsigned long)rxBuffer & 0xff;     // die ersten 8 Bits enthalten eine Zufalls-ID (wird beim Batteriewechsel neu generiert)
        bool bState = (unsigned long)(rxBuffer >> 8) & 0x1; // wenn Bit 8 gesetzt ist, sind die Batterien schwach
        byte xBits = (unsigned long)(rxBuffer >> 9) & 0x3;  // die Bits 9 und 10 sind bei Wind- und Regensensor immer 1 (Wert von xBits = 3)
        buttonState = (unsigned long)rxBuffer >> 11 & 0x1;  // wenn Bit 11 gesetzt ist, wurde die Taste am Sensor betaetigt
        /*
     *  Hinweis!
     *  Weil sich die Zufalls-ID bei jedem Batteriewechsel aendert, werden nur die xBits (sind immer gesetzt),
     *  das Bit 4 der Zufalls-ID (ist immer Null) und die subID benutzt, um die richtigen Sensoren zu finden.
     *  Das sollte ausreichen, solange sich nicht eine gleiche Wetterstation in Reichweite befindet.
     */
        if (!(randomID & 0x10))
        { // Bit 4 der Zufalls-ID ist 0
            DEBUG_PRINT(F("RandomID: "));
            DEBUG_PRINT(randomID);
            DEBUG_PRINT(F(" xBits: "));
            DEBUG_PRINT(xBits);
            DEBUG_PRINT(F(" bState: "));
            DEBUG_PRINT(bState);
            DEBUG_PRINT(F(" buttoState: "));
            DEBUG_PRINTLN(buttonState);

            if (xBits < 3)
            { // xBits < 3 ist Temperatur oder Luftfeuchtigkeit
                if (calculateChecksume(0xf, false))
                {
                    if (pairingEndMillis != 0)
                    {
                        pairingDevice(randomID, xBits);
                    }

                    if ((!pairingRequired) || (isPaired(randomID)))
                    {
                        bState ? batteryState |= 1 : batteryState &= 0; // wenn die Batterien schwach sind, Bit 0 von batteryState auf 1 setzen

                        temperature = ((unsigned long)(rxBuffer >> 12) & 0xfff);                              // die Bits 12-23 enthalten den Temperaturwert (in 0.1 °C)
                        humidity = ((unsigned long)((rxBuffer >> 24) & 0xf) * 10) + ((rxBuffer >> 28) & 0xf); // die Bits 24-27 (einzer) und 28-31 (zehner) enthalten den Luftfeuchtigkeitsert (in %)

                        DEBUG_PRINTF("Temperatur: %d.%d", (int)(temperature / 10), (int)(temperature % 10));
                        DEBUG_PRINT("°C")
                        DEBUG_PRINTF("\r\nLuftfeuchtigkeit: %d%%\r\n", humidity);

                        newData = 'T';
                    }
                }
                else
                {
                    DEBUG_PRINTLN(F("checksume wrong for temperature and humidity"));
                }
            }
            else if (xBits == 3)
            {                                                       // xBits = 3 ist Wind- oder Regensensor
                byte subID = (unsigned long)(rxBuffer >> 12) & 0x7; // die Bits 12, 13, 14 enthalten eine Sub-ID (Windmesser sendet 2 Telegramme Sub-ID 1 und 7)
                DEBUG_PRINT(F("SubID: "));
                DEBUG_PRINTLN(subID);

                if (pairingEndMillis != 0)
                {
                    pairingDevice(randomID, xBits, subID);
                }

                if ((!pairingRequired) || (isPaired(randomID)))
                {
                    if ((subID == 1) || (subID == 7))
                    {
                        if (calculateChecksume(0xf, false))
                        {
                            if (subID == 1)
                            {                                                             // subID = 1 ist der Windsensor (Durchschnitts-Windgeschwindigkeit)
                                bState ? batteryState |= 1 : batteryState &= 0;           // wenn die Batterien schwach sind, Bit 0 von batteryState auf 1 setzen
                                windSpeed = ((unsigned long)(rxBuffer >> 24) & 0xff) * 2; // die Bits 24-31 enthalten die durchschnittliche Windgeschwindigkeit in
                                                                                          // Einheiten zu 0.2 m/s und mal 10 ergibt: "* 2" fuer einen Integerwert
                                DEBUG_PRINTF("Windgeschwindigkeit (/): %d.%dm/s\r\n", (int)(windSpeed / 10), (int)(windSpeed % 10));

                                newData = 'S';
                            }
                            if (subID == 7)
                            {                                                            // subID = 7 ist der Windsensor (Windrichtung und Windboen)
                                bState ? batteryState |= 1 : batteryState &= 0;          // wenn die Batterien schwach sind, Bit 0 von batteryState auf 1 setzen
                                uint16_t wdir = (unsigned long)(rxBuffer >> 15) & 0x1ff; // die Bits 15-23 enthalten die Windrichtung in Grad (0-360)
                                if (wdir <= 360)
                                    windDirection = wdir;                                // die Windrichtung wird manchmal falsch uebertragen, deshalb nur Daten bis 360 Grad uebernehmen
                                windGust = ((unsigned long)(rxBuffer >> 24) & 0xff) * 2; // die Bits 24-31 enthalten die Windboen in
                                //uint16_t wg = ((unsigned long) (rxBuffer >> 24) & 0xff) * 2; // die Bits 24-31 enthalten die Windboen in
                                // Einheiten zu 0.2 m/s und mal 10 ergibt: "* 2" fuer einen Integerwert
                                //if (wg < 500) windGust = wg;  // die Windboen werden manchmal falsch uebertragen, deshalb nur Daten unter 50m/s (180km/h) uebernehmen
                                DEBUG_PRINTF("Windrichtung: %3d", windDirection);
                                DEBUG_PRINT("°");
                                DEBUG_PRINTF("\r\nWindboen: %d.%dm/s\r\n", (int)(windGust / 10), (int)(windGust % 10));

                                newData = 'G';
                            }
                        }
                        else
                        {
                            DEBUG_PRINTLN(F("checksume wrong for wind sensor"));
                        }
                    }
                    else if (subID == 3)
                    { // subID = 3 ist der Regensensor (absolute Regenmenge seit Batteriewechsel)
                        if (calculateChecksume(0x7, true))
                        {
                            bState ? batteryState |= 2 : batteryState &= 0;               // wenn die Batterien schwach sind, Bit 1 von batteryState auf 1 setzen
                            rainVolume = ((unsigned long)(rxBuffer >> 16) & 0xffff) * 25; // die Bits 16-31 enthalten die Niederschlagsmenge
                                                                                          // in Einheiten zu je 0.25 mm -> 1 mm = 1 L/m2
                                                                                          // hier zusaetzlich mal 10 um einen Integerwert zu erhalten
                            DEBUG_PRINTF("Regenmenge: %d.%dmm\r\n", (int)(rainVolume / 100), (int)(rainVolume % 100));

                            newData = 'R';
                        }
                        else
                        {
                            DEBUG_PRINTLN(F("checksume wrong for rain sensor"));
                        }
                    }
                }
            }
        }
        rxBuffer = 0; // den Puffer loeschen
        rxOk = false; // Auswertung beendet, dann rxOk zuruecksetzen fuer die Interrupt-Routine
    }

    return newData;
}

float WeatherStationDataRx::readTemperature(bool inF)
{
    float temp = (float)temperature / 10;
    return inF ? convertCtoF(temp) : temp;
}

uint8_t WeatherStationDataRx::readHumidity()
{
    return humidity;
}

float WeatherStationDataRx::readWindSpeed(bool inKMH)
{
    return inKMH ? (float)windSpeed * 36 / 100 : (float)windSpeed / 10;
}

uint16_t WeatherStationDataRx::readWindDirection()
{
    return windDirection;
}

float WeatherStationDataRx::readWindGust(bool inKMH)
{
    return inKMH ? (float)windGust * 36 / 100 : (float)windGust / 10;
}

float WeatherStationDataRx::readRainVolume()
{
    return (float)rainVolume / 100;
}

bool WeatherStationDataRx::readButtonState()
{
    return buttonState;
}

float WeatherStationDataRx::convertCtoF(float c)
{
    return c * 1.8 + 32;
}

float WeatherStationDataRx::convertFtoC(float f)
{
    return (f - 32) * 0.55555;
}

bool WeatherStationDataRx::calculateChecksume(byte startValue, bool add)
{
    unsigned long checksumCalc = startValue;
    for (byte n = 0; n < 32; n += 4)
    {
        DEBUG_PRINT((unsigned long)((rxBuffer >> n) & 0xf));
        DEBUG_PRINT(" ");

        if (add)
        {
            checksumCalc += ((rxBuffer >> n) & 0xf);
        }
        else
        {
            checksumCalc -= ((rxBuffer >> n) & 0xf);
        }
    }
    checksumCalc &= 0xf;
    unsigned long checksum = (rxBuffer >> 32) & 0xf;
    DEBUG_PRINTF("Checksum calc=%lu read=%lu\r\n", checksumCalc, checksum);

    return checksumCalc == checksum;
}

bool WeatherStationDataRx::isPaired(byte randomID)
{
    for (int i = 0; i < pairedDevicesCount; i++)
    {
        if (pairedDevices[i] == randomID)
            return true;
    }
    return false;
}

bool WeatherStationDataRx::pairingDevice(byte randomID, byte xBits, byte subID)
{
    if (isPaired(randomID))
        return false;

    pairedDevices = (byte *)realloc(pairedDevices, ++pairedDevicesCount * sizeof(byte));
    pairedDevices[pairedDevicesCount - 1] = randomID;
    DEBUG_PRINTF("New %s device with ID %d paired\r\n", (xBits < 3 ? "temperature" : (subID == 1 ? "wind speed" : (subID == 7 ? "wind direction and gust" : (subID == 3 ? "rain sensor" : "unknown")))), randomID);

    if (pairedDeviceAdded != NULL)
        pairedDeviceAdded(randomID);

    return true;
}

byte WeatherStationDataRx::batteryStatus() {
  return batteryState;
}

byte WeatherStationDataRx::sensorID() {
  return randomID;
}