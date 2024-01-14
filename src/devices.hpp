#ifndef DEVICES_HPP
#define DEVICES_HPP

#include "drivers/serial/ISerial.hpp"

extern SerialComm::ISerial* pMainSerial;

void initDevices();

#endif