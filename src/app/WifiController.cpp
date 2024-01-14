#include "WifiController.hpp"
#include <Arduino.h>
#include <ESP8266HTTPClient.h>

#include "utilities/print/Print.hpp"

const static uint32_t CONNECT_TIMEOUT_MS = 60000;
const static uint32_t CONNECT_CHECK_TIME_MS = 500;

namespace Wifi
{
    void dumpResult(const char* result);

    WifiController::WifiController()
    {
        pServer_ = nullptr;
        clientHandler_ = nullptr;
    }
    void WifiController::shutOff()
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
    PRINTLN("Connecting to '%s'", ssid);
#endif

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
#ifdef DEBUG
            PRINT(".");
#endif
            ESP.wdtFeed();
            delay(CONNECT_CHECK_TIME_MS);
        }
#ifdef DEBUG
        PRINT("\n");
        PRINTLN("Conn %s", success ? "PASS" : "FAIL");
#endif
        return success;
    }

    void WifiController::disconnect()
    {
        WiFi.mode(WIFI_OFF);
        WiFi.forceSleepBegin();
        delay(1);
    }

    uint16_t WifiController::get(
        const char* url,
        uint16_t port,
        uint8_t* response,
        uint16_t respLen)
    {
        uint16_t numBytes = 0;
        if (!isConnected())
        {
            return numBytes;
        }

#ifdef LOCAL_SERVER
        WiFiClient client;
#else
        WiFiClientSecure client;
        client.setInsecure();
#endif
        client.connect(url, port);

        // Start communication with server
        HTTPClient http;
        http.begin(client, url);

        // Post the data
        int statusCode = http.GET();
        if (statusCode == HTTP_SUCCESS)
        {
            const char* httpPayload = http.getString().c_str();
            numBytes = strlen(httpPayload) + 1;
            if (numBytes > respLen) numBytes = respLen;
            memcpy(response, httpPayload, numBytes);
        }

        // End transaction
        http.end();

#ifdef DEBUG
        PRINTLN("Received code %d", statusCode);
#endif
        return numBytes;
    }

    uint16_t WifiController::post(
        const char* url,
        uint16_t port,
        const uint8_t* payload,
        uint16_t payloadLen,
        uint8_t* response,
        uint16_t respLen)
    {
        uint16_t numBytes = 0;
        if (!isConnected())
        {
            return numBytes;
        }

#ifdef LOCAL_SERVER
        WiFiClient client;
#else
        WiFiClientSecure client;
        client.setInsecure();
#endif
        client.connect(url, port);

        // Start communication with server
        HTTPClient http;
        http.begin(client, url);

        // Post the data
        int statusCode = http.POST((uint8_t*)payload, payloadLen);
        if ((statusCode == HTTP_SUCCESS) &&
            (response != nullptr) &&
            (respLen > 0))
        {
            const char* httpPayload = http.getString().c_str();
            numBytes = strlen(httpPayload) + 1;
            if (numBytes > respLen) numBytes = respLen;
            memcpy(response, httpPayload, numBytes);
        }

        // End transaction
        http.end();

#ifdef DEBUG
        PRINTLN("Received code %d", statusCode);
#endif
        return numBytes;
    }

    bool WifiController::startServer(uint16_t port, void (*clientHandler)(WiFiClient*))
    {
        if (pServer_ != nullptr)
        {
            stopServer();
        }

        if (!isConnected())
        {
            return false;
        }

        clientHandler_ = clientHandler;
        pServer_ = new WiFiServer(port);

        pServer_->begin();

#ifdef DEBUG
        PRINTLN("Server open on %s:%d", WiFi.localIP().toString().c_str(), pServer_->port());
#endif

        return true;
    }

    void WifiController::stopServer()
    {
        pServer_->close();

        clientHandler_ = nullptr;
        delete(pServer_);
        pServer_ = nullptr;
    }

    void WifiController::updateServer()
    {
        if (!pServer_ || !clientHandler_ || (!isConnected()))
        {
            return;
        }

        WiFiClient client = pServer_->available();
        if (!client)
        {
            return;
        }

#ifdef DEBUG
        PRINTLN("Client connected.");
#endif

        while (!client.available())
        {
            delay(10);
            ESP.wdtFeed();
        }
        
        clientHandler_(&client);
    }

    void WifiController::skipHeader(WiFiClient* pClient)
    {
        // Skip the header
        String line = pClient->readStringUntil('\n');
        uint16_t strLen = line.length();
        while (strLen > 0)
        {
            if (line.equals("\r"))
            {
                // This indicates the start of a body
                break;
            }
            line = pClient->readStringUntil('\n');
            strLen = line.length();
        }
    }

    void WifiController::sendGenericSuccessHeader(WiFiClient* pClient)
    {
        pClient->println("HTTP/1.1 200 OK");
        pClient->println("Content-Type: application/json");
        pClient->println("");
    }

    void dumpResult(const char* result)
    {
        uint16_t resultLen = strlen(result);
        int16_t bytesToPrint = resultLen;
        char tmpBuffer[MAX_PRINT_LENGTH+1] = {0};
        while (bytesToPrint > 0)
        {
            int16_t bytesToCopy = (bytesToPrint > MAX_PRINT_LENGTH) ?
                MAX_PRINT_LENGTH :
                bytesToPrint;

            memcpy(tmpBuffer, &(result[resultLen - bytesToPrint]), bytesToCopy);
            PRINT(tmpBuffer);
            bytesToPrint -= bytesToCopy;

            ESP.wdtFeed();
            delay(1000);
        }
        PRINTLN("");
    }
}