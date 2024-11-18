#include "SetupHandler.hpp"
#include <ESP8266WiFi.h>
#include "utilities/print/Print.hpp"
#include "config.hpp"
#include "devices.hpp"

const static char* SUCCESS_HEADER = "HTTP/1.1 200 OK\r\n";
const static char* BAD_CLIENT_HEADER = "HTTP/1.1 404 Not Found\r\n";
const static char* BAD_SERVER_HEADER = "HTTP/1.1 500 Internal Server Error\r\n";
const static char* CONTENT_TYPE = "Content-Type: application/json\r\n";
const static char* CONTENT_LENGTH = "Content-Length: %d\r\n";
const static char* HEADER_END = "\r\n";
const static char* GENERIC_SUCCESS_BODY = "{\"success\":true}";
const static char* GENERIC_FAIL_BODY = "{\"success\":false}";
const static char* ID_BODY = "{\"success\":true,\"id\":%d}";
const static char* STATUS_BODY = "{\"success\":true,\"status\":%d}";
const static char* SCAN_BODY_OPEN = "{\"success\":true,\"ssids\":[";
const static char* SCAN_BODY_CLOSE = "]}";
const static char* REQ_TYPE     = "GET"; 


struct SetupCommand
{
    const char* name;
    void (*function)(WiFiClient*, String);
};

static void handleIdRequest    (WiFiClient* pClient, String request);
static void handleScanRequest  (WiFiClient* pClient, String request);
static void handleWifiRequest  (WiFiClient* pClient, String request);
static void handleStatusRequest(WiFiClient* pClient, String request);
static void handleExitRequest  (WiFiClient* pClient, String request);

const static SetupCommand setupCommands[] =
{
    {.name = "/id",     .function = &handleIdRequest},
    {.name = "/scan",   .function = &handleScanRequest},
    {.name = "/wifi",   .function = &handleWifiRequest},
    {.name = "/status", .function = &handleStatusRequest},
    {.name = "/exit",   .function = &handleExitRequest}
};
const static uint8_t NUM_SETUP_COMMANDS = sizeof(setupCommands) / sizeof(setupCommands[0]);

void handleIdRequest(WiFiClient* pClient, String request)
{
    pClient->print(SUCCESS_HEADER);
    pClient->print(CONTENT_TYPE);
    pClient->print(HEADER_END);
    DEBUG_PRINTLN("ID: %d", pApp->getHopperId());

    if (pApp->getHopperId() <= 0)
    {
        pClient->print(GENERIC_FAIL_BODY);
        return;
    }

    pClient->printf(ID_BODY, pApp->getHopperId());
}

void handleScanRequest(WiFiClient* pClient, String request)
{
    (void)request;

    uint8_t numNetworks = WiFi.scanNetworks();

    pClient->print(SUCCESS_HEADER);
    pClient->print(CONTENT_TYPE);
    pClient->print(HEADER_END);

    bool firstAp = true;
    pClient->print(SCAN_BODY_OPEN);
    for (uint8_t i=0; i<numNetworks; i++)
    {
        pClient->printf("%c{\"ssid\":\"%s\", \"rssi\":%d}",
                        firstAp ? ' ' : ',',
                        WiFi.SSID(i).c_str(),
                        WiFi.RSSI(i));
        DEBUG_PRINTLN("%s: %d", WiFi.SSID(i).c_str(), WiFi.RSSI(i));

        firstAp = false;
    }
    pClient->print(SCAN_BODY_CLOSE);
    pClient->stop();
}

void handleWifiRequest(WiFiClient* pClient, String request)
{
    char ssid[MAX_SSID_LEN+1] = {0};
    char password[MAX_PASSWORD_LEN+1] = {0};
    int result = sscanf(request.c_str(), "/wifi?ssid=%[^&]&pass=%s", ssid, password);
    DEBUG_PRINTLN("S: %s | P: %s (%d)", ssid, password, result);

    if (result < 1)
    {
        // Send bad request
        pClient->print(BAD_CLIENT_HEADER);
        pClient->print(CONTENT_TYPE);
        pClient->print(HEADER_END);
        pClient->print(GENERIC_FAIL_BODY);
        pClient->stop();
        return;
    }

    pClient->print(SUCCESS_HEADER);
    pClient->print(CONTENT_TYPE);
    pClient->printf(CONTENT_LENGTH, strlen(GENERIC_SUCCESS_BODY));
    pClient->print(HEADER_END);
    pClient->print(GENERIC_SUCCESS_BODY);
    pClient->stop();

    pApp->testConnect(ssid, password);
}

void handleStatusRequest(WiFiClient* pClient, String request)
{
    (void)request;
    
    pClient->print(SUCCESS_HEADER);
    pClient->print(CONTENT_TYPE);
    pClient->print(HEADER_END);
    pClient->printf(STATUS_BODY, WiFi.status());
    pClient->stop();
}

void handleExitRequest(WiFiClient* pClient, String request)
{
    (void)request;
    
    pClient->print(SUCCESS_HEADER);
    pClient->print(CONTENT_TYPE);
    pClient->printf(CONTENT_LENGTH, strlen(GENERIC_SUCCESS_BODY));
    pClient->print(HEADER_END);
    pClient->print(GENERIC_SUCCESS_BODY);
    pClient->stop();

    pApp->exitSetup();
}

void clientHandler(WiFiClient* pClient)
{
    // Only need the request type and path
    String requestType = pClient->readStringUntil(' ');
    String request = pClient->readStringUntil(' ');
    
    // Read all the client's data
    unsigned long startTime = millis();
    while (pClient->available())
    {
       pClient->read();

       if ((millis() - startTime) > 5000)
       {
            DEBUG_PRINTLN("CLIENT TIMEOUT");
            return;
       }
    }
    
    DEBUG_PRINTLN("AP REC:");
    DEBUG_PRINTLN(requestType.c_str());
    DEBUG_PRINTLN(request.c_str());

    if (strncmp(requestType.c_str(), REQ_TYPE, strlen(REQ_TYPE)) != 0)
    {
        pClient->print(BAD_CLIENT_HEADER);
        pClient->print(CONTENT_TYPE);
        pClient->print(HEADER_END);
        pClient->print(GENERIC_FAIL_BODY);
        pClient->stop();
        return;
    }

    for (uint8_t i=0; i<NUM_SETUP_COMMANDS; i++)
    {
        if (strncmp(request.c_str(), setupCommands[i].name, strlen(setupCommands[i].name)) == 0)
        {
            setupCommands[i].function(pClient, request);
            return;
        }
    }

    // Request not recognized
    pClient->print(BAD_CLIENT_HEADER);
    pClient->print(CONTENT_TYPE);
    pClient->print(HEADER_END);
    pClient->print(GENERIC_FAIL_BODY);
    pClient->stop();
}