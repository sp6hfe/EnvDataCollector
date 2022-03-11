#include <Arduino.h>

#include "application.h"
#include "config.h"
#include "httpUploader.h"
#include "hwBme280.h"
#include "sensorHumidity.h"
#include "sensorPressureRaw.h"
#include "sensorTemperature.h"
#include "wifiCore.h"

/* wifi */
wrappers::WifiCore wifiCore(Serial, config::web_server_port);

/* sensors */
wrappers::HwBme280 bme280;
sensors::SensorTemperature sensorTemperature(bme280, "BME280 temp", "*C",
                                             "temperature");
sensors::SensorHumidity sensorHumidity(bme280, "BME280 humi", "%", "humidity");
sensors::SensorPressureRaw sensorPressureRaw(bme280, "BME280 pres", "hPa",
                                             "pressure_raw");

/* uploaders */
uploaders::HttpUploader httpUploader(Serial, wifiCore, "http");

/* app */
application::Application app(Serial, wifiCore);

void setup() {
#ifdef DEBUG
  // delay in debug to allow PlatformIO terminal to start
  // after code uploading is finished
  delay(1500);
#endif

  Serial.begin(config::console_baudrate);
  httpUploader.begin(config::upload_path, config::api_key, config::token);

  app.registerSensor(&sensorTemperature);
  app.registerSensor(&sensorHumidity);
  app.registerSensor(&sensorPressureRaw);
  app.registerUploader(&httpUploader);

  if (!app.setup()) {
    Serial.println("Application setup failed. Investigate log for failures.");
  }
}

void loop() { app.loop(millis()); }
