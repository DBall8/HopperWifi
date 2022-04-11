#ifndef WIFI_CONTROLLER_HPP
#define WIFI_CONTROLLER_HPP

#include <stdint.h>

namespace Wifi
{
    enum class WifiStatus : uint8_t
    {
        UNCONNECTED = 0,
        CONNECTED,

    };

    class WifiController
    {
        public:
            WifiController();
            ~WifiController(){}
            void init();
            bool connect(const char* ssid, const char* password);

        private:
            WifiStatus status_;
    };
}

#endif