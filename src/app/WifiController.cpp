#include "WifiController.hpp"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

const static uint32_t CONNECT_TIMEOUT_MS = 60000;
const static uint32_t CONNECT_CHECK_TIME_MS = 500;

namespace Wifi
{
    WifiController::WifiController()
    {

    }
    void WifiController::init()
    {
        // Put in low power mode until something happens
        WiFi.mode(WIFI_OFF);
        WiFi.forceSleepBegin();
        delay(1);
    }

    bool WifiController::connect(const char* ssid, const char* password)
    {
        bool success = false;

        WiFi.forceSleepWake();
        delay(1);

#ifdef DEBUG
        

        // Setup wifi
        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);

        // Wait until we are connected
        uint16_t numConnectLoops = CONNECT_TIMEOUT_MS / CONNECT_CHECK_TIME_MS;
        for (uint16_t i=0; i<numConnectLoops; i++)
        {
            if ((WiFi.isConnected()))
            {
                // Connected!
                success = true;
                break;
            }
            delay(CONNECT_CHECK_TIME_MS);
        }

        return success;
    }
}