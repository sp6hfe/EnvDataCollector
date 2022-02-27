#pragma once

namespace config {

/* WiFi section */
static const char *ssid = "wifi";
static const char *pass = "*#ky3orrmotita!";
static const uint8_t wifi_connect_timeout_sec = 10;

/* AP section */
static const char *ap_ssid = "EnvDataCollector";
static const char *ap_pass = "*#Env0123!";

/* web server section */
static constexpr int web_server_port = 80;

/* data upload section */
static const char *upload_path =
    "http://192.168.1.222/www/envdatacollector/upload";
static const char *api_key = "667F6C3E9D911C79B0806E5F88D34C57";
static const char *token = "3E4FB8E8CC45214F16FC58D8C6410D80";

/* app section */
static constexpr uint16_t console_baudrate = 9600;
static constexpr uint8_t inter_measurements_delay_sec = 60;

}  // namespace config
