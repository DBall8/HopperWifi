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
            HopperSocket(Wifi::WifiController* pWifi);
            ~HopperSocket(){}

            void registerSocket();
            bool connect(int32_t hopperId, const char* ssid, const char* password);
            void disconnect();
            void update();
            bool isConnected();
            void log(const char* logStr);

            void handleMessage(socketIOmessageType_t type, uint8_t * payload, size_t length);

        private:
            const static uint16_t STR_BUFF_LEN = 1024;
            char strBuff_[STR_BUFF_LEN+1] = {0};

            Wifi::WifiController* pWifi_;
            SocketIOclient webSocket_;
            bool wifiConnected_ = false;
            bool socketConnected_ = false;
            int32_t hopperId_ = -1;

            void storeString(const char* str, size_t len);
            bool parseMessage(char** pName, char** pData);

            void sendMessage(const char* cmd, const char* payload);
            void sendMessage(const char* cmd, const uint32_t payload);

            void handleConnect(const char* msg, size_t len);
            void handleEvent(char* name, char* data);
            void handleId();
            void handleStatus();
            void handleCmd(const char* cmd);
            void handleCal(const char* step);
            void handleTest(const char* msg, size_t len);
            void sendFail(const char* name);
            void sendPass(const char* name);
    };
}

#endif