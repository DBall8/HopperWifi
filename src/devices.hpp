#ifndef DEVICES_HPP
#define DEVICES_HPP

#include "drivers/serial/ISerial.hpp"
#include "app/WifiApp.hpp"

extern SerialComm::ISerial* pMainSerial;
extern Hopper::WifiApp* pApp;

void initDevices();

#endif