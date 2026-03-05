#include <Arduino.h>
#include <M5Core2.h>

#include "RS232Manager.h"
#include "DebugManager.h"
#include "ConfigManager.h"

HardwareSerial SerialRS232(2);

RS232Manager rs232(SerialRS232);
DebugManager debug;
ConfigManager config;

void setup() {

    M5.begin();

    debug.begin(115200);

    config.load();

    rs232.begin(config.baud_rate);

    debug.log("Systeme demarre");

}

void loop() {

}