#pragma once

#include <Stream.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

namespace wrappers
{
    class WifiCore
    {
    private:
        Stream &console;
        WiFiClient wifi_client;
        HTTPClient http_client;
        ESP8266WebServer web_server;

    public:
        void restart() { ESP.restart(); };

        void wifiBegin();
        bool wifiConnect(const char *wifi_ssid, const char *wifi_passphrase, const uint8_t timeout_sec);
        IPAddress wifiGetIp() { return WiFi.localIP(); };

        bool apBegin(const char *ssid, const char *psk, int channel, int ssid_hidden, int max_connection) { return WiFi.softAP(ssid, psk, channel, ssid_hidden, max_connection); };
        IPAddress apGetIp() { return WiFi.softAPIP(); };

        void webserverBegin() { web_server.begin(); };
        void webserverRegisterPage(const char *uri, std::function<void(void)> callback) { web_server.on(uri, callback); };
        const String &webserverGetArg(const String &name) { return web_server.arg(name); };
        void webserverSend(int code, const char *content_type, const String &content) { web_server.send(code, content_type, content); };
        void webserverPerform() { web_server.handleClient(); };

        bool httpBegin(const String &url) { return this->http_client.begin(wifi_client, url); };
        void httpAddHeader(const String &name, const String &value, bool first = false, bool replace = true) { this->http_client.addHeader(name, value, first, replace); };
        int httpSendPost(const String &payload) { return this->http_client.POST(payload); };
        void httpEnd() { this->http_client.end(); };

        WifiCore(Stream &console_, uint16_t web_server_port_) : console(console_), web_server(web_server_port_){};
    };

    void WifiCore::wifiBegin()
    {
        // cleaning after previous configuration that may be still active
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        WiFi.persistent(false);
    }

    bool WifiCore::wifiConnect(const char *wifi_ssid, const char *wifi_passphrase, const uint8_t timeout_sec)
    {
        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(wifi_ssid, wifi_passphrase);

        auto timeout_counter = 0;
        while (WiFi.status() != WL_CONNECTED && timeout_counter < timeout_sec)
        {
            this->console.print(".");
            delay(1000);
            ++timeout_counter;
        }

        return (WiFi.status() == WL_CONNECTED);
    }
} // namespace wrappers