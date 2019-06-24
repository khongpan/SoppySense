#include <Arduino.h>
#include "Network.h"

void setup() {

    Net.Setup();
}

void loop() {

    Net.PowerOn();
    Net.Connect();
    Net.HttpGet();
    Net.Disconnect();
    Net.PowerOff();
    
    delay(5000);
  
}