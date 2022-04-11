#include <Arduino.h>

const static char* SSID = "TitansTower";
const static char* PASS = "seaslug29";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("ALIVE!");
  delay(1000);
}