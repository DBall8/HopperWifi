#ifndef WIFI_APP_HPP
#define WIFI_APP_HPP

#include "app/HopperSocket.hpp"
#include "app/WifiController.hpp"

namespace Hopper
{
    enum AppState
    {
        UNDEFINED = 0,
        NORMAL,
        SETUP
    };

    class WifiApp
    {
        public:
            WifiApp(Wifi::WifiController* pWifi, HopperSocket* pSocket);
            ~WifiApp(){}

            void loadConfigs();
            void start();
            AppState getState();
            void update();

            bool isSocketConnected();
            void log(const char* message);

            void enterSetup();
            void exitSetup();
            bool testConnect(const char* ssid, const char* password);

            int32_t getHopperId(){ return hopperId_; }

        private:
            Wifi::WifiController* pWifi_;
            HopperSocket* pSocket_;

            char ssid_[MAX_SSID_LEN] = {0};
            char password_[MAX_PASSWORD_LEN] = {0};
            int32_t hopperId_ = -1;

            HopperTimer socketRetryTimer_;
            AppState state_;
            WiFiServer* pSetupServer_;

            void startSocket();
    };
}
#endif