#include "WifiCli.hpp"
#include "hopper_shared.hpp"
#include "devices.hpp"
#include "app/EepromManager.hpp"
#include "drivers/serial/ISerial.hpp"
#include "utilities/print/Print.hpp"
#include "app/HopperSocket.hpp"

using namespace Cli;
using namespace SerialComm;
using namespace Hopper;

extern ISerial* pMainSerial;

static void echoCmd(uint16_t argc, ArgV argv)
{
    PRINTLN(ECHO_RESP_STR);
}

static void wifiCmd(uint16_t argc, ArgV argv)
{
    if (argc == 2)
    {
        // Only SSID given
        setWifiCreds(argv[1], nullptr);
    }
    else if (argc == 3)
    {
        // SSID and password given
        setWifiCreds(argv[1], argv[2]);
    }
    else
    {
        // Wrong number of inputs
        PRINTLN(FAIL_STR);
        return;
    }

    PRINTLN(PASS_STR);
    HopperSocket::getInstance().loadConfigs();
    HopperSocket::getInstance().connect();
}

static void idCmd(uint16_t argc, ArgV argv)
{
    if (argc != 2)
    {
        PRINTLN(FAIL_STR);
        return;
    }

    int32_t id = atoi(argv[1]);

    if ((id < 0) || (id > UINT16_MAX))
    {
        PRINTLN(FAIL_STR);
        return;
    }

    setId((uint16_t)id);
    PRINTLN(PASS_STR);

    HopperSocket::getInstance().loadConfigs();
    HopperSocket::getInstance().connect();
}

static void clearCmd(uint16_t argc, ArgV argv)
{
    clearWifiCreds();
    PRINTLN(PASS_STR);
}

static void statusCmd(uint16_t argc, ArgV argv)
{
    if (HopperSocket::getInstance().isConnected())
    {
        PRINTLN(CONNECTED_STR);
    }
    else
    {
        PRINTLN(DISCONNECTED_STR);
    }
}

static void getCmd(uint16_t argc, ArgV argv)
{
    PRINTLN("%d", getId());
}

static void logCmd(uint16_t argc, ArgV argv)
{
    if (argc >= 2)
    {
        HopperSocket::getInstance().log(argv[1]);
    }
}

static void setupCmd(uint16_t argc, ArgV argv)
{
    PRINTLN("START AP MODE\n");
    return;
}

const static Command commands[] =
{
    {.name = ECHO_CMD_STR, .function = &echoCmd},
    {.name = WIFI_CMD_STR, .function = &wifiCmd},
    {.name = ID_CMD_STR, .function = &idCmd},
    {.name = CLEAR_CMD_STR, .function = &clearCmd},
    {.name = MAIN_STATUS_CMD_STR, .function = &statusCmd},
    {.name = GET_CMD_STR, .function = &getCmd},
    {.name = LOG_CMD_STR, .function = &logCmd},
    {.name = SETUP_CMD_STR, .function = &setupCmd},
};
const static uint8_t NUM_COMMANDS = sizeof(commands) / sizeof(commands[0]);

static CommandInterface wifiCli(pMainSerial, commands, NUM_COMMANDS, true);
CommandInterface* pCli = &wifiCli;