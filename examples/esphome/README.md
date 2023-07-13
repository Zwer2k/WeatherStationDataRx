# ESPHome configuration

- copy this file [WeatherStationSensor.h](WeatherStationSensor.h) to yourESPHome project folder.
- copy & paste the below example into ESPHome. Change it by your configuration.

The application need to know wind_station_id / rain_station_id. We can switch it to scan mode (set wind_station_id=0 and
rain_station_id=0). After that we can check logs and when we find sensor records with real values, take sensor ID and
save it to wind_station_id / rain_station_id.

```yaml
substitutions:
  patform: "esp32"
  board: "nodemcu-32s" 
  device_name: weatherstation
  comment: "Meteostation"
  ip: "192.168.1.21"
  station_pin: "27"   # GPIO
  wind_station_id: "0"  # 0 = scan all available sensors (we can check it in logs). When we find our wind sensor, set its ID instead "0" 
  rain_station_id: "0"  # 0 = scan all available sensors (we can check it in logs). When we find our rain sensor, set its ID instead "0" 


esphome:
  name: '${device_name}'
  comment: '${comment}'
  platform: '${patform}'
  board: "${board}"
  libraries:
    - https://github.com/Zwer2k/WeatherStationDataRx.git@0.5.0
  includes:
    - WeatherStationSensor.h

# Enable logging
logger:
#  level: DEBUG #VERY_VERBOSE

ota:
  password: "123456789123456789"

wifi:
  use_address: "${ip}"
  ssid: !secret wifi_ssid
  password: !secret wifi_password

  # Enable fallback hotspot (captive portal) in case wifi connection fails
  ap:
    ssid: "Weather-Station Fallback Hotspot"
    password: "ZFPdpx2JP4O8"

mqtt:
  broker: 192.168.1.1
  username: mqtt_user
  password: !secret mqtt_password  


sensor:
  - platform: custom
    lambda: |-
      WeatherSensor *ws = new WeatherSensor(${station_pin}, ${wind_station_id}, ${rain_station_id});
      App.register_component(ws);
      return {
        ws->temperature,
        ws->humidity,
        ws->wind_speed,
        ws->wind_direction,
        ws->wind_gust,
        ws->rain_volume,
        ws->battery_wind_station,
        ws->battery_rain_station
      };      
    sensors:
      - name: "${device_name} Temperature"
        id: "${device_name}_temperature"
        device_class: TEMPERATURE
        state_class: measurement
        unit_of_measurement: "°C"
        accuracy_decimals: 2
        force_update: true
        filters:
          - quantile:
              quantile: .25

      - name: "${device_name} Humidity"
        id: "${device_name}_humidity"
        device_class: HUMIDITY
        state_class: measurement
        unit_of_measurement: "%"
        accuracy_decimals: 2
        force_update: true
        filters:
          - quantile:
              quantile: .25

      - name: "${device_name} Wind Speed"
        id: "${device_name}_wind_speed"
        device_class: WIND_SPEED
        state_class: measurement
        unit_of_measurement: m/s
        accuracy_decimals: 2
        force_update: true


      - name: "${device_name} Wind Direction"
        id: "${device_name}_wind_direction"
        state_class: measurement
        unit_of_measurement: °
        accuracy_decimals: 0
        force_update: true


      - name: "${device_name} Wind Gust"
        id: "${device_name}_wind_gust"
        device_class: WIND_SPEED
        state_class: measurement
        unit_of_measurement: m/s
        accuracy_decimals: 2
        force_update: true


      - name: "${device_name} Rain volume"
        id: "${device_name}_rain_volume"
        device_class: WATER
        state_class: total_increasing
        unit_of_measurement: L
        accuracy_decimals: 2
        force_update: true
        filters:
          - quantile:
              send_first_at: 3
              quantile: .0025


      - name: "${device_name} Battery wind station"
        id: "${device_name}_battery_wind_station"
        device_class: BATTERY
        state_class: measurement
        unit_of_measurement: "%"
        accuracy_decimals: 0
        force_update: false

      - name: "${device_name} Battery rain station"
        id: "${device_name}_battery_rain_station"
        device_class: BATTERY
        state_class: measurement
        unit_of_measurement: "%"
        accuracy_decimals: 0
        force_update: false

```
