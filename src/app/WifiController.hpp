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

            bool startServer(uint16_t port, void (*clientHandler)(WiFiClient*));
            void stopServer();
            void updateServer();

            void skipHeader(WiFiClient* pClient);
            void sendGenericSuccessHeader(WiFiClient* pClient);

        private:
            WiFiServer* pServer_;
            void (*clientHandler_)(WiFiClient*);
    };
}

#endif