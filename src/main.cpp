#include <Arduino.h>
#include "application.h"
#include "config.h"
#include "hwBme280.h"
#include "sensorHumidity.h"
#include "sensorPressureRaw.h"
#include "sensorTemperature.h"
#include "wifiCore.h"

/* sensors */
wrappers::HwBme280 bme280;
sensors::SensorTemperature sensorTemperature(bme280, "temperature", "*C");
sensors::SensorHumidity sensorHumidity(bme280, "humidity", "%");
sensors::SensorPressureRaw sensorPressureRaw(bme280, "pressure_raw", "hPa");

/* wifi */
wrappers::WifiCore wifiCore(Serial, config::web_server_port);

/* app */
application::Application app(Serial, wifiCore);

void setup() {
#ifdef DEBUG
  // delay in debug to allow PlatformIO terminal to start
  // after code uploading is finished
  delay(1500);
#endif

  Serial.begin(config::console_baudrate);
  Serial.println();
  app.registerSensor(&sensorTemperature);
  app.registerSensor(&sensorHumidity);
  app.registerSensor(&sensorPressureRaw);
  if (!app.setup()) {
    Serial.println("Application setup failed. Investigate log for failures.");
  }
}

void loop() { app.loop(millis()); }
