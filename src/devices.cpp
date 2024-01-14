#include "devices.hpp"
#include "drivers/serial/arduino/ArduinoSerial.hpp"
#include "utilities/print/Print.hpp"

using namespace SerialComm;

const static uint16_t SERIAL_BAUD_RATE = 9600;
static ArduinoSerial serial(SERIAL_BAUD_RATE);
ISerial* pMainSerial = &serial;

void initDevices()
{
    serial.initialize();
    PrintHandler::getInstance().initialize(&serial);
}