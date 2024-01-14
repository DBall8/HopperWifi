#include "HopperSocket.hpp"
#include "hopper_shared.hpp"
#include "utilities/print/Print.hpp"
#include "devices.hpp"
#include "utilities/circular_queue/CircularQueue.hpp"

#include <ESP8266WiFi.h>
#include <string.h>

namespace Hopper
{

#ifdef LOCAL_SERVER
    const static char* URL = "192.168.0.17";
    //const static char* TEST_URL = "http://192.168.0.14:8002/test";
    const static uint16_t PORT = 8002;
#else
    const static char* URL = "veranus.site";
    //const static char* TEST_URL = "https://veranus.site/test";
    const static uint16_t PORT = 443;
#endif
    const static char* PATH = "/feeder_control/?EIO=3";
    //const static char* PATH = "/feeder_control/?transport=websocket";
    const static char* MSG_FORMAT_STR = "[\"%s\",\"%s\"]";
    const static char* MSG_FORMAT_NUM = "[\"%s\",%d]";

    const static char* ID_STR = "id";
    const static char* ID_R_STR = "id_r";
    const static char* STATUS_STR = "status";
    const static char* STATUS_R_STR = "status_r";
    const static char* CMD_STR = "cmd";
    const static char* CMD_R_STR = "cmd_r";
    const static char* LOG_STR = "log";

    #define MAX_LOG_LEN 31
    #define MAX_LOG_Q_LEN 10
    static char logBuffer[MAX_LOG_LEN * MAX_LOG_Q_LEN] = {0};
    static uint8_t currLogIndex = 0;

    static void eventHandler(socketIOmessageType_t type, uint8_t * payload, size_t length)
    {
        HopperSocket::getInstance().handleMessage(type, payload, length);
    }

    HopperSocket& HopperSocket::getInstance()
    {
        static HopperSocket instance;
        return instance;
    }

    HopperSocket::HopperSocket():
        wifiRetryTimer_(RETRY_MS)
    {}

    void HopperSocket::copyString(const char* str, size_t len)
    {
        if (len >= STR_BUFF_LEN)
        {
#ifdef DEBUG
            PRINTLN("CONN msg too large.");
#endif
            return;
        }

        memset(strBuff_, 0, STR_BUFF_LEN);
        memcpy(strBuff_, str, len);
    }

    bool HopperSocket::parseMessage(char** pName, char** pData)
    {
        char* msg = strBuff_;

        char* name = nullptr;
        char* data = nullptr;
        int8_t nameLen = 0;
        int8_t dataLen = 0;
        int8_t currLen = strlen(strBuff_);
        
        if ((msg[0] != '[') || (msg[currLen-1] != ']'))
        {
            // Bad formatting
            return false;
        }

        // Removing surrounding brackets
        msg[currLen-1] ='\0';
        msg++;
        currLen -= 2;
        if (currLen <= 0)
        {
            return false;
        }
        
        // Find the separating comma
        char* pComma = strchr(msg, ',');
        if (pComma != nullptr) *pComma = '\0';
        
        name = msg;
        nameLen = strlen(name);

        // Remove quotes
        if ((name[0] != '\"' ) || (name[nameLen-1] != '\"'))
        {
            return false;
        }
        name[nameLen-1] = '\0';
        name++;
        nameLen -= 2;

        if (pComma == nullptr)
        {
            *pName = name;
            return true;
        }

        // Data is located after the comma
        data = pComma+1;
        dataLen = strlen(data);

        if ((data[0] == '\"' ) && (data[dataLen-1] == '\"'))
        {
            // Is a string
            data[dataLen-1] = '\0';
            data++;
            dataLen -= 2;
        }
        else if ((data[0] != '\"' ) && (data[dataLen-1] != '\"'))
        {
            // Is a number
            // TODO
        }
        else
        {
            return false;
        }

        *pName = name;
        *pData = data;

        return true;
    }

    void HopperSocket::sendMessage(const char* cmd, const char* payload)
    {
        memset(strBuff_, 0, STR_BUFF_LEN);
        uint8_t numBytes = snprintf(strBuff_, STR_BUFF_LEN, MSG_FORMAT_STR, cmd, payload);

        if (numBytes >= STR_BUFF_LEN)
        {
            // Can't be sure we were able to copy the entire string
#ifdef DEBUG
        PRINTLN("SENDING FAILED");
#endif
            return;
        }

#ifdef DEBUG
        PRINTLN("SENDING:");
        PRINTLN(strBuff_);
#endif

        webSocket_.sendEVENT(strBuff_);
    }

    void HopperSocket::sendMessage(const char* cmd, const uint32_t payload)
    {
        memset(strBuff_, 0, STR_BUFF_LEN);
        uint8_t numBytes = snprintf(strBuff_, STR_BUFF_LEN, MSG_FORMAT_NUM, cmd, payload);

        if (numBytes >= (STR_BUFF_LEN))
        {
            // Can't be sure we were able to copy the entire string
#ifdef DEBUG
        PRINTLN("SENDING FAILED");
#endif
            return;
        }

#ifdef DEBUG
        PRINTLN("SENDING:");
        PRINTLN(strBuff_);
#endif

        webSocket_.sendEVENT(strBuff_);
    }

    void HopperSocket::handleConnect(const char* msg, size_t len)
    {
        copyString(msg, len);
        PRINTLN("Rec %d bytes:", len);
        PRINTLN(strBuff_);
    }

    void HopperSocket::handleEvent(char* name, char* data)
    {
        if (strcmp(name, ID_STR) == 0)
        {
            handleId();
        }
        else if (strcmp(name, STATUS_STR) == 0)
        {
            handleStatus();
        }
        else if (strcmp(name, CMD_STR) == 0)
        {
            handleCmd(data);
        }
    }

    void HopperSocket::handleId()
    {
        if (hopperId_ > 0)
        {
            sendMessage(ID_R_STR, hopperId_);
        }

        if (currLogIndex > 0)
        {
            // Send next available log
            for (uint8_t i=0; i<currLogIndex; i++)
            {
                webSocket_.loop();
                sendMessage(LOG_STR, &(logBuffer[i*MAX_LOG_LEN]));
            }
            memset(logBuffer, 0, MAX_LOG_LEN*MAX_LOG_Q_LEN);
            currLogIndex = 0;
        }
    }

    void HopperSocket::handleStatus()
    {
        const static uint8_t MAX_STATUS_LEN = 8;
        pMainSerial->flush();
        PRINTLN(STATUS_CMD);

        memset(strBuff_, 0, STR_BUFF_LEN);
        uint8_t numBytes = pMainSerial->readLine((uint8_t*)strBuff_, STR_BUFF_LEN, CMD_TIMEOUT_MS);

        if (numBytes <= 0)
        {
            sendFail(STATUS_R_STR);
            return;
        }

        // Remove trailing whitespace
        for (uint8_t i=numBytes-1; i>=0; i--)
        {
            if ((strBuff_[i] == '\n') ||
                (strBuff_[i] == '\r') ||
                (strBuff_[i] == ' '))
            {
                strBuff_[i] = '\0';
            }
            else if (strBuff_[i] != '\0')
            {
                break;
            }
        }

        if (strlen(strBuff_) > MAX_STATUS_LEN)
        {
            sendFail(STATUS_R_STR);
            return;
        }

        char payload[MAX_STATUS_LEN+1] = {0};
        memcpy(payload, strBuff_, strlen(strBuff_));

        sendMessage(STATUS_R_STR, payload);
    }

    void HopperSocket::handleCmd(const char* cmd)
    {
        const char* cmdStr = nullptr;
        char successStr[5] = {0};
        int status = 3;

        if (cmd == nullptr)
        {
            sendFail(CMD_R_STR);
            return;
        }

        if (strcmp(cmd, OPEN_CMD) == 0)
        {
            cmdStr = OPEN_CMD;
        }
        else if (strcmp(cmd, CLOSE_CMD) == 0)
        {
            cmdStr = CLOSE_CMD;
        }
        else
        {
            sendFail(CMD_R_STR);
            return;
        }

        pMainSerial->flush();
        PRINTLN(cmdStr);

        memset(strBuff_, 0, STR_BUFF_LEN);
        if (!pMainSerial->readLine((uint8_t*)strBuff_, STR_BUFF_LEN, CMD_TIMEOUT_MS))
        {
            sendFail(CMD_R_STR);
            return;
        }

        uint8_t numTokens = sscanf(strBuff_, "%s %d", successStr, &status);
        if ((numTokens == 2) &&
            (strcmp(successStr, PASS_STR) == 0) &&
            (status >= 0))
        {
            sendMessage(CMD_R_STR, status);
        }
        else
        {
            sendFail(CMD_R_STR);
        }
    }

    void HopperSocket::handleMessage(socketIOmessageType_t type, uint8_t * payload, size_t length)
    {
        char* name = nullptr;
        char* data = nullptr;

        switch (type)
        {
        case socketIOmessageType_t::sIOtype_CONNECT:
#ifdef DEBUG
            PRINTLN("Socket connected.");
#endif
            break;

        case socketIOmessageType_t::sIOtype_EVENT:
            copyString((char*)payload, length);

#ifdef DEBUG
            PRINTLN("REC event: %d bytes", length);
            PRINTLN(strBuff_);
#endif 
            if (!parseMessage(&name, &data) ||
                (name == nullptr))
            {
                return;
            }

            handleEvent(name, data);
            break;

        case socketIOmessageType_t::sIOtype_ERROR:
#ifdef DEBUG
            PRINTLN("REC: ERR");
            copyString((char*)payload, length);
            PRINTLN("REC: %d bytes", length);
            PRINTLN(strBuff_);
#endif
            break;

        case socketIOmessageType_t::sIOtype_DISCONNECT:
            break;
        
        default:
            break;
        }
    }

    void HopperSocket::loadConfigs()
    {
        getWifiCreds(ssid_, password_);
        hopperId_ = getId();

    #ifdef DEBUG
        PRINTLN("\n");
        PRINTLN("ID: %d", hopperId_);
        PRINTLN("SSID: %s", ssid_);
        PRINTLN("PASS: %s", password_);
    #endif
    }

    void HopperSocket::connect()
    {
        // Stop the retry timer, and only restart it if we have valid configs
        wifiRetryTimer_.stop();

        if (wifiConnected_ || socketConnected_)
        {
            disconnect();
        }

        if ((ssid_ != nullptr) &&
            (ssid_[0] != '\0') &&
            (hopperId_ > 0))
        {
            // First, connect to an AP
            wifi_.shutOff();
            if (!wifi_.connect(ssid_, password_))
            {
#ifdef DEBUG
                PRINTLN("CONN FAILED");
#endif
                wifiConnected_ = false;
                wifiRetryTimer_.start();
                return;
            }

            wifiConnected_ = true;
            DEBUG_PRINTLN("CONNECTED");

            webSocket_.setReconnectInterval(500);
#ifdef LOCAL_SERVER
            webSocket_.begin(URL, PORT, PATH);
#else
            webSocket_.beginSSL(URL, PORT, PATH);
#endif
            // Set event handler
            webSocket_.onEvent(eventHandler);
            webSocket_.enableHeartbeat(HEARTBEAT_MS, HEARTBEAT_TIMEOUT_MS, DC_HEARTBEAT_COUNT);
        }
    }

    void HopperSocket::disconnect()
    {
        if (socketConnected_)
        {
            webSocket_.disconnect();
        }

        if (wifiConnected_)
        {
            wifiConnected_ = false;
            wifi_.disconnect();
        }
    }

    void HopperSocket::update()
    {
        bool wifiConnNew = wifi_.isConnected();
        if (wifiConnected_ != wifiConnNew)
        {
            wifiConnected_ = wifiConnNew;
            if (!wifiConnected_) connect();
        }
        else if (!wifiConnected_)
        {
            retryConnection();
        }
        else
        {
            webSocket_.loop();

            bool isSocketConn = webSocket_.isConnected();
            if (socketConnected_ != isSocketConn)
            {
                PRINTLN(isSocketConn ? CONNECTED_STR : DISCONNECTED_STR);
                socketConnected_ = isSocketConn;

                if (!socketConnected_) webSocket_.setReconnectInterval(SOCKET_RECONNECT_MS);
            }
        }
    }
            
    bool HopperSocket::isConnected()
    {
        return socketConnected_;
    }

    void HopperSocket::log(const char* logStr)
    {
        if (isConnected())
        {
            sendMessage(LOG_STR, logStr);
        }
        else
        {
            if (currLogIndex >= MAX_LOG_Q_LEN) return;

            DEBUG_PRINTLN("QUEUING LOG");
            uint16_t logLen = strlen(logStr);
            uint16_t bytesToCopy = (logLen > MAX_LOG_LEN) ? MAX_LOG_LEN : logLen;
            memcpy(&(logBuffer[currLogIndex*MAX_LOG_LEN]), logStr, bytesToCopy);
            currLogIndex++;
        }
    }

    void HopperSocket::sendFail(const char* name)
    {
        sendMessage(name, "fail");
    }

    void HopperSocket::retryConnection()
    {
        if (wifiRetryTimer_.hasPeriodPassed())
        {
            connect();
        }
    }
}