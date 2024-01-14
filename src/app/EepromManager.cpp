#include "EepromManager.hpp"
#include <EEPROM.h>
#include "utilities/print/Print.hpp"

namespace Hopper
{
    // Define EEPROM space and offsets
    const static uint16_t EEPROM_INIT_FLAG = 0xBEEF;
    const static uint8_t EEPROM_SIZE = MAX_SSID_LEN + MAX_PASSWORD_LEN + ID_LEN + sizeof(EEPROM_INIT_FLAG);
    const static uint8_t EEPROM_SSID_OFFSET = 0;
    const static uint8_t EEPROM_PASSWORD_OFFSET = EEPROM_SSID_OFFSET + MAX_SSID_LEN;
    const static uint8_t EEPROM_ID_OFFSET = EEPROM_PASSWORD_OFFSET + MAX_PASSWORD_LEN;
    const static uint8_t EEPROM_INIT_FLAG_OFFSET = EEPROM_ID_OFFSET + ID_LEN;

    static bool eepromInitialized = false;

    void clearEeprom()
    {
        DEBUG_PRINTLN("CLEAR");
        for (uint8_t i=EEPROM_SSID_OFFSET; i<EEPROM_INIT_FLAG_OFFSET; i++)
        {
            EEPROM.write(i, 0);
        }
        EEPROM.write(EEPROM_INIT_FLAG_OFFSET, (uint8_t)(EEPROM_INIT_FLAG >> 8));
        EEPROM.write(EEPROM_INIT_FLAG_OFFSET + 1, (uint8_t)(EEPROM_INIT_FLAG));
        EEPROM.commit();
    }

    void initEeprom()
    {
        EEPROM.begin(EEPROM_SIZE);
        if ((EEPROM.read(EEPROM_INIT_FLAG_OFFSET) != (uint8_t)(EEPROM_INIT_FLAG >> 8)) ||
            (EEPROM.read(EEPROM_INIT_FLAG_OFFSET+1) != (uint8_t)(EEPROM_INIT_FLAG)))
        {
            DEBUG_PRINTLN("1: %x", EEPROM.read(EEPROM_INIT_FLAG_OFFSET));
            DEBUG_PRINTLN("2: %x", EEPROM.read(EEPROM_INIT_FLAG_OFFSET+1));
            clearEeprom();
        }
        eepromInitialized = true;
    }

    void getWifiCreds(char* ssid, char* password)
    {
        if (!eepromInitialized) initEeprom();

        if (ssid == nullptr) return;

        uint8_t i;
        for (i=0; i<MAX_SSID_LEN; i++)
        {
            ssid[i] = EEPROM.read(EEPROM_SSID_OFFSET + i);

            if (ssid[i] == '\0')
            {
                // End of string found
                break;
            }
        }

        if (password == nullptr) return;
        
        for (i=0; i<MAX_PASSWORD_LEN; i++)
        {
            password[i] = EEPROM.read(EEPROM_PASSWORD_OFFSET + i);

            if (password[i] == '\0')
            {
                // End of string found
                break;
            }
        }
    }

    uint16_t getId()
    {
        if (!eepromInitialized) initEeprom();

        uint16_t id = 0;

        for (uint8_t i=0; i<ID_LEN; i++)
        {
            id |= (uint8_t)(EEPROM.read(EEPROM_ID_OFFSET + i) << (8*i));
        }

        return id;
    }

    void setWifiCreds(char* ssid, char* password)
    {
        if (!eepromInitialized) initEeprom();

        if (ssid == nullptr) return;

        uint8_t i;
        for (i=0; i<MAX_SSID_LEN; i++)
        {
            EEPROM.write(EEPROM_SSID_OFFSET + i, (uint8_t)ssid[i]);

            if (ssid[i] == '\0')
            {
                // Last byte written
                break;
            }
        }

        if (password != nullptr)
        {
            for (i=0; i<MAX_PASSWORD_LEN; i++)
            {
                EEPROM.write(EEPROM_PASSWORD_OFFSET + i, (uint8_t)password[i]);

                if (password[i] == '\0')
                {
                    // Last byte written
                    break;
                }
            }
        }

        EEPROM.commit();
    }

    void clearWifiCreds()
    {
        uint8_t i;
        for (i=0; i<MAX_SSID_LEN; i++)
        {
            EEPROM.write(EEPROM_SSID_OFFSET + i, 0);
        }

        for (i=0; i<MAX_PASSWORD_LEN; i++)
        {
            EEPROM.write(EEPROM_PASSWORD_OFFSET + i, 0);
        }

        EEPROM.commit();
    }

    void setId(uint16_t id)
    {
        if (!eepromInitialized) initEeprom();

        for (uint8_t i=0; i<ID_LEN; i++)
        {
            EEPROM.write(EEPROM_ID_OFFSET + i, (uint8_t)(id >> (i*8)));
        }

        EEPROM.commit();
    }
}