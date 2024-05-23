#ifndef WIFI_CONTROLLER_HPP
#define WIFI_CONTROLLER_HPP

#include <stdint.h>
#include <ESP8266WiFi.h>
#include "HopperTimer.hpp"

namespace Wifi
{
    const static uint8_t HTTP_SUCCESS = 200;

    class WifiController
    {
        public:
            WifiController();
            ~WifiController(){}
            void shutOff();
            bool isConnected() { return WiFi.isConnected(); }
            bool connect(const char* ssid, const char* password);
            void disconnect();
            void startAp(const char* name, const char* password);
            void startAp(const char* name,
                         const char* password,
                         IPAddress localIp,
                         IPAddress gatewayIp,
                         IPAddress subnetIp);

            uint16_t get(const char* url,
                        uint16_t port,
                        uint8_t* response,
                        uint16_t respLen);

            uint16_t post(const char* url,
                          uint16_t port,
                          const uint8_t* payload,
                          uint16_t payloadLen,
                          uint8_t* response = nullptr,
                          uint16_t respLen = 0);

            WiFiServer* startServer(uint16_t port);
            void stopServer(WiFiServer* pServer);
            void updateServer(WiFiServer* pServer, void (*clientHandler)(WiFiClient*));

            void skipHeader(WiFiClient* pClient);
            void sendGenericSuccessHeader(WiFiClient* pClient);
    };
}

#endif