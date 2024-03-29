#include <Arduino.h>

#include "application.h"
#include "config.h"
#include "esp8266Core.h"
#include "httpUploader.h"
#include "hwBme280.h"
#include "sensorHumidity.h"
#include "sensorPressureRaw.h"
#include "sensorTemperature.h"
#include "webConfigurator.h"

/* core */
wrappers::ESP8266Core esp8266Core(Serial, config::web_server_port);

/* sensors */
wrappers::HwBme280 bme280;
sensors::SensorTemperature sensorTemperature(bme280, "BME280 temp", "*C",
                                             "temperature");
sensors::SensorHumidity sensorHumidity(bme280, "BME280 humi", "%", "humidity");
sensors::SensorPressureRaw sensorPressureRaw(bme280, "BME280 pres", "hPa",
                                             "pressure_raw");

/* uploaders */
uploaders::HttpUploader httpUploader(Serial, esp8266Core, "http");

/* configurator */
configurators::WebConfigurator webConfigurator(Serial, esp8266Core,
                                               esp8266Core);

/* app */
application::Application app(Serial, esp8266Core);

void setup() {
#ifdef DEBUG
  // delay in debug to allow PlatformIO terminal to start
  // after code uploading is finished
  delay(1500);
#endif

  Serial.begin(config::console_baudrate);
  httpUploader.begin(config::ssid, config::pass,
                     config::wifi_connect_timeout_sec, config::upload_path,
                     config::api_key, config::token);
  webConfigurator.begin(config::ap_ssid, config::ap_pass);

  app.setInterMeasurementsDelay(config::inter_measurements_delay_sec);
  app.registerSensor(&sensorTemperature);
  app.registerSensor(&sensorHumidity);
  app.registerSensor(&sensorPressureRaw);
  app.registerUploader(&httpUploader);
  app.registerConfigurator(&webConfigurator);

  if (!app.setup()) {
    Serial.println("Application setup failed. Investigate log for failures.");
  }
}

void loop() { app.loop(millis()); }
