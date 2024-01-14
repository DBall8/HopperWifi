#include <Arduino.h>
#include "devices.hpp"
#include "app/WifiCli.hpp"
#include "utilities/print/Print.hpp"
#include "app/EepromManager.hpp"
#include "app/HopperSocket.hpp"

#include "app/HopperSocket.hpp"

using namespace Hopper;

void test();

void setup() {
  initDevices();
  pCli->enable();
  ESP.wdtEnable(WDTO_8S);

  HopperSocket::getInstance().loadConfigs();
  HopperSocket::getInstance().connect();
}

void loop() {
  // put your main code here, to run repeatedly:
  // PRINTLN("ALIVE!");
  delay(10);
  ESP.wdtFeed();
  pCli->update();

  HopperSocket::getInstance().update();
}