#include "config.h"
#include <Adafruit_BME280.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <InfluxDb.h>
#include <Wire.h>

Adafruit_BME280 bme;
WiFiClient client;
Influxdb influx(String(influxdb_host));
// http://cactusprojects.com/esp8266-logging-to-influxdb/
// https://randomnerdtutorials.com/esp8266-web-server/

void measure();

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("**********");
  Serial.println("Sensor Data Collector by SP6HFE");
  Serial.println("**********");
  Serial.println();

  // bool status = bme.begin(0x76);
  bool status = bme.begin(BME280_ADDRESS_ALTERNATE);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }
  bme.setSampling(Adafruit_BME280::MODE_FORCED, Adafruit_BME280::SAMPLING_X16,
                  Adafruit_BME280::SAMPLING_X16, Adafruit_BME280::SAMPLING_X16,
                  Adafruit_BME280::FILTER_OFF, Adafruit_BME280::STANDBY_MS_0_5);
  Serial.print("BME280 sensor detected.");
  Serial.println();

  Serial.print("Connecting to WiFi AP: \"");
  Serial.print(ssid);
  Serial.print("\"");

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting measurements.");
}

void loop() {
  static constexpr unsigned long INTER_MEASUREMENTS_DELAY_SEC = 30;

  measure();
  delay(INTER_MEASUREMENTS_DELAY_SEC * 1000);
}

void measure() {
  if (bme.takeForcedMeasurement()) {
    Serial.print(bme.readTemperature());
    Serial.print(" [*C], ");
    Serial.print(bme.readHumidity());
    Serial.print(" [%], ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" [hPa]");
  } else {
    Serial.println("Forced measurement failed.");
  }
}