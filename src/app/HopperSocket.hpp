#ifndef HOPPER_SOCKET_HPP
#define HOPPER_SOCKET_HPP

#include <stdint.h>
#include <SocketIOclient.h>
#include "WifiController.hpp"
#include "EepromManager.hpp"
#include "HopperTimer.hpp"
#include "config.hpp"

namespace Hopper
{
    class HopperSocket
    {
        public:
            static HopperSocket& getInstance();

            void loadConfigs();
            void connect();
            void disconnect();
            void update();
            bool isConnected();
            void log(const char* logStr);

            void handleMessage(socketIOmessageType_t type, uint8_t * payload, size_t length);

        private:
            const static uint16_t STR_BUFF_LEN = 1024;
            char strBuff_[STR_BUFF_LEN+1] = {0};

            char ssid_[MAX_SSID_LEN] = {0};
            char password_[MAX_PASSWORD_LEN] = {0};
            int32_t hopperId_ = -1;

            Wifi::WifiController wifi_;
            SocketIOclient webSocket_;
            HopperTimer wifiRetryTimer_;
            bool wifiConnected_ = false;
            bool socketConnected_ = false;

            void retryConnection();
            void copyString(const char* str, size_t len);
            bool parseMessage(char** pName, char** pData);

            void sendMessage(const char* cmd, const char* payload);
            void sendMessage(const char* cmd, const uint32_t payload);

            void handleConnect(const char* msg, size_t len);
            void handleEvent(char* name, char* data);
            void handleId();
            void handleStatus();
            void handleCmd(const char* cmd);
            void handleTest(const char* msg, size_t len);
            void sendFail(const char* name);

            HopperSocket();
            ~HopperSocket(){}
            HopperSocket (const HopperSocket&) = delete;
            HopperSocket& operator= (const HopperSocket&) = delete;
    };
}

#endif