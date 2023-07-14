#include "esphome.h"
#include <WeatherStationDataRx.h>



class WeatherSensor : public Component, public Sensor {

public:
  WeatherStationDataRx wsdr;
  Sensor *temperature = new Sensor();
  Sensor *humidity = new Sensor();
  Sensor *wind_speed = new Sensor();
  Sensor *wind_direction = new Sensor();
  Sensor *wind_gust = new Sensor();
  Sensor *rain_volume = new Sensor();
  Sensor *battery_wind_station = new Sensor();
  Sensor *battery_rain_station = new Sensor();
  byte wind_station_id = 0;
  byte rain_station_id = 0;

  WeatherSensor(byte pin, byte wind_id, byte rain_id) : wsdr(pin, false, ARMIgnore, false), wind_station_id(wind_id), rain_station_id(rain_id) {}

  void setup() override {
     wsdr.begin();
     if (wind_station_id > 0 && rain_station_id > 0) {
       byte myDeviceIDs[] = {wind_station_id, rain_station_id};
       wsdr.pair(myDeviceIDs, sizeof(myDeviceIDs));
       ESP_LOGI("INFO", "Wind station id: %d \n", wind_station_id);
       ESP_LOGI("INFO", "Rain station id: %d \n", rain_station_id);
     }
  }

  void loop() override {
    byte newDataState = wsdr.readData(true);
    if (newDataState == 0)
       return;
    byte sensorID = wsdr.sensorID();
    if (wind_station_id == 0 || sensorID == wind_station_id) {
      if (wsdr.dataHas(newDataState, NDTemperature)) {
        float temper = wsdr.readTemperature();
        if (wind_station_id > 0) {
          temperature->publish_state(temper);
          byte batt = bitRead(wsdr.batteryStatus(), 0) == 0 ? 100 : 0;
          battery_wind_station->publish_state(batt);
        } else {
          ESP_LOGW("INFO", "Get temperature %f C  (device_id %d)", temper, sensorID);
        }
      }

      if (wsdr.dataHas(newDataState, NDHumidity)) {
        uint8_t humi = wsdr.readHumidity();
        if (wind_station_id > 0) {
          humidity->publish_state(humi);
        } else {
          ESP_LOGW("INFO", "Get humidity %f % (device_id %d)", humi, sensorID);
        }
      }

      if (wsdr.dataHas(newDataState, NDWindSpeed)) {
        float win_s = wsdr.readWindSpeed();
        if (wind_station_id > 0) {
          wind_speed->publish_state(win_s);
        } else {
          ESP_LOGW("INFO", "Get wind speed %f m/s (device_id %d)", win_s, sensorID);
        }
      }

      if (wsdr.dataHas(newDataState, NDWindDirection)) {
        uint16_t win_d = wsdr.readWindDirection();
        if (wind_station_id > 0) {
          wind_direction->publish_state(win_d);
        } else {
          ESP_LOGW("INFO", "Get wind direction %d (device_id %d)", win_d, sensorID);
        }
      }

      if (wsdr.dataHas(newDataState, NDWindGust)) {
        float  win_g = wsdr.readWindGust();
        if (wind_station_id > 0) {
          wind_gust->publish_state(win_g);
        } else {
          ESP_LOGW("INFO", "Get wind gust %f m/s (device_id %d)", win_g, sensorID);
        }
      }
    }
    if (rain_station_id == 0 || sensorID == rain_station_id) {
      if (wsdr.dataHas(newDataState, NDRainVolume)) {
        float rain = wsdr.readRainVolume();
        if (rain_station_id > 0) {
          rain_volume->publish_state(rain);

          byte batt = wsdr.batteryStatus() == 0 ? 100 : 5;
          battery_rain_station->publish_state(batt);
        } else {
          ESP_LOGW("INFO", "Get rain volume %f mm (device_id %d)", rain, sensorID);
        }
      }
    }
    if (wsdr.dataHas(newDataState, NDError)) {
      ESP_LOGE("INFO", "pairing required");
    }
 }
};
