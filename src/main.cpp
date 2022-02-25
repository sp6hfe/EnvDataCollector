#include <Arduino.h>
#include "config.h"
#include "wifiCore.h"
#include "hwBme280.h"

enum class OpMode
{
  RESTART,
  MEASUREMENTS,
  CONFIG,
};

wrappers::WifiCore wifi_core(Serial, 80);
wrappers::HwBme280 bme280;

OpMode op_mode = OpMode::RESTART;

bool upload();
void log_measurements();
void configure_web_server();
void root_web_page();
void config_web_page();
void restart_web_page();

void setup()
{
#ifdef DEBUG
  delay(1000);
#endif
  static constexpr uint8_t WIFI_CONNECT_DELAY_SEC = 10;

  wifi_core.wifiBegin();

  Serial.begin(9600);
  Serial.println();
  Serial.println("**********");
  Serial.println("Environmental Data Collector by SP6HFE");
  Serial.println("**********");
  Serial.println();

  if (bme280.init())
  {
    Serial.println("BME280 sensor detected.");
  }
  else
  {
    Serial.println("Could not detect a BME280 sensor, check wiring and device address!");
    while (1)
      ;
  }

  Serial.print("Connecting to WiFi AP: \"");
  Serial.print(config::ssid);
  Serial.print("\"");

  if (wifi_core.wifiConnect(config::ssid, config::pass, WIFI_CONNECT_DELAY_SEC))
  {
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(wifi_core.wifiGetIp());
    Serial.println("Starting measurements.");
    op_mode = OpMode::MEASUREMENTS;
  }
  else
  {
    Serial.println();
    Serial.println("WiFi connection timeout - starting AP.");
    if (wifi_core.apBegin("test", "testtest", 5, 0, 2))
    {
      Serial.println("");
      Serial.print("AP IP: ");
      Serial.println(wifi_core.apGetIp());
      Serial.println("Starting web server.");
      configure_web_server();
      wifi_core.webserverBegin();
      Serial.println("Done.");
      op_mode = OpMode::CONFIG;
    }
    else
    {
      Serial.println("AP setup failed. There is no way to communicate wirelessly.");
      op_mode = OpMode::RESTART;
    }
  }
}

void loop()
{
  static constexpr unsigned long INTER_MEASUREMENTS_DELAY_SEC = 60;

  switch (op_mode)
  {
  case OpMode::MEASUREMENTS:
    if (bme280.measure())
    {
      log_measurements();
      if (!upload())
      {
        Serial.println("Error on data uploading.");
      }
    }
    else
    {
      Serial.println("Error on taking measurements.");
    }
    delay(INTER_MEASUREMENTS_DELAY_SEC * 1000);
    break;
  case OpMode::CONFIG:
    wifi_core.webserverPerform();
    break;
  default:
    /* fall though */
  case OpMode::RESTART:
    Serial.println("Restarting...");
    wifi_core.restart();
    break;
  }
}

bool float_to_ints(float val, int &int_part, int &frac_part, int frac_size)
{
  if (frac_size <= 0)
  {
    return false;
  }

  int_part = static_cast<int>(val);
  float temp_val = (val - int_part);
  for (int i = 0; i < frac_size + 1; i++)
  {
    temp_val *= 10;
  }
  frac_part = static_cast<int>(temp_val);
  if ((frac_part % 10) >= 5)
  {
    frac_part++;
  }
  frac_part /= 10;

  return true;
}

bool float_to_char(float val, int frac_size, char *buffer)
{
  bool if_converted = false;
  int val_int, val_frac;

  if (float_to_ints(val, val_int, val_frac, frac_size))
  {
    sprintf(buffer, "%d.%d", val_int, val_frac);
    if_converted = true;
  }

  return if_converted;
}

bool upload()
{
  static constexpr int FRACT_SIZE = 2;
  bool if_uploaded = false;
  char conversion_buffer[8]; // max length is pressure: '1020.57' + '\0'

  if (wifi_core.httpBegin(config::upload_path))
  {
    wifi_core.httpAddHeader("Content-Type", "application/x-www-form-urlencoded");

    String data_to_upload = "api_key=";
    data_to_upload += config::api_key;
    data_to_upload += "&token=";
    data_to_upload += config::token;
    data_to_upload += "&temperature=";
    if (float_to_char(bme280.getTemperature(), FRACT_SIZE, conversion_buffer))
    {
      data_to_upload += conversion_buffer;
      data_to_upload += "&humidity=";
      if (float_to_char(bme280.getHumidity(), FRACT_SIZE, conversion_buffer))
      {
        data_to_upload += conversion_buffer;
        data_to_upload += "&pressure_raw=";
        if (float_to_char(bme280.getPressureRaw(), FRACT_SIZE, conversion_buffer))
        {
          data_to_upload += conversion_buffer;

#ifdef DEBUG
          Serial.println();
          Serial.print(data_to_upload);
#endif

          int post_response = wifi_core.httpSendPost(data_to_upload);
          if (post_response == 200)
          {
            if_uploaded = true;
          }
#ifdef DEBUG
          else
          {
            Serial.print(" ");
            Serial.print(post_response);
            Serial.print(" ");
          }
#endif
        }
      }
    }

    wifi_core.httpEnd();
  }

  return if_uploaded;
}

void log_measurements()
{
  Serial.print(bme280.getTemperature());
  Serial.print(" [*C], ");
  Serial.print(bme280.getHumidity());
  Serial.print(" [%], ");
  Serial.print(bme280.getPressureRaw());
  Serial.print(" [hPa]");
}

String st;
String content;
int statusCode;

void configure_web_server()
{
  wifi_core.webserverRegisterPage("/", root_web_page);
  wifi_core.webserverRegisterPage("/config", config_web_page);
  wifi_core.webserverRegisterPage("/restart", restart_web_page);
}

void root_web_page()
{
  Serial.println("Client has accessed main page.");
  IPAddress ip = wifi_core.apGetIp();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) +
                 '.' + String(ip[3]);
  content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
  content += ipStr;
  content += "<p>";
  content += st;
  content += "</p><form method='get' action='setting'><label>SSID: "
             "</label><input name='ssid' length=32><input name='pass' "
             "length=64><input type='submit'></form>";
  content += "</html>";
  wifi_core.webserverSend(200, "text/html", content);
}

void config_web_page()
{
  String new_ssid = wifi_core.webserverGetArg("ssid");
  String new_pass = wifi_core.webserverGetArg("pass");

  if (new_ssid.length() > 0 && new_pass.length() > 0)
  {
    Serial.println("Received data:");
    Serial.print("ssid: ");
    Serial.println(new_ssid);
    Serial.print("pass: ");
    Serial.println(new_pass);
    content = "{\"Success\":\"Data received.\"}";
    statusCode = 200;
  }
  else
  {
    Serial.println("Received data not usable. Sending 404.");
    content = "{\"Error\":\"404 not found\"}";
    statusCode = 404;
  }
  wifi_core.webserverSend(statusCode, "application/json", content);
}

void restart_web_page()
{
  Serial.println("Self reset. Bye!");
  content = "{\"Success\":\"Self reset. Bye!\"}";
  wifi_core.webserverSend(200, "application/json", content);
  delay(1000);
  wifi_core.restart();
}