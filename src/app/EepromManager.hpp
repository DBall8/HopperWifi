#ifndef EEPROM_MANAGER_HPP
#define EEPROM_MANAGER_HPP

#include <stdint.h>

namespace Hopper
{
    const static uint8_t MAX_SSID_LEN = 32;
    const static uint8_t MAX_PASSWORD_LEN = 64;
    const static uint8_t ID_LEN = sizeof(uint16_t);

    void initEeprom();
    void getWifiCreds(char* ssid, char* password);
    uint16_t getId();

    void setWifiCreds(char* ssid, char* password);
    void clearWifiCreds();
    void setId(uint16_t id);
}

#endif