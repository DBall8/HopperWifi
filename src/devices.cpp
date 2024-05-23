#include "devices.hpp"
#include "drivers/serial/arduino/ArduinoSerial.hpp"
#include "utilities/print/Print.hpp"

using namespace SerialComm;
using namespace Wifi;
using namespace Hopper;

const static uint16_t SERIAL_BAUD_RATE = 9600;
static ArduinoSerial serial(SERIAL_BAUD_RATE);
ISerial* pMainSerial = &serial;

static WifiController wifiController;
static HopperSocket hopperSocket(&wifiController);

static WifiApp wifiApp(&wifiController, &hopperSocket);
WifiApp* pApp = &wifiApp;

void initDevices()
{
    serial.initialize();
    PrintHandler::getInstance().initialize(&serial);
}