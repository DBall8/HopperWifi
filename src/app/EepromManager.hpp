#ifndef EEPROM_MANAGER_HPP
#define EEPROM_MANAGER_HPP

#include <stdint.h>

namespace Hopper
{
    void initEeprom();
    void eeprom_getWifiCreds(char* ssid, char* password);
    uint16_t eeprom_getId();

    void eeprom_setWifiCreds(const char* ssid, const char* password);
    void eeprom_clearWifiCreds();
    void eeprom_setId(uint16_t id);
}

#endif