#include "RS232Manager.h"

RS232Manager::RS232Manager(HardwareSerial& port) {
    serial = &port;
}

void RS232Manager::begin(int baud) {
    baudRate = baud;
    serial->begin(baudRate, SERIAL_8N1, 13, 14);
}

void RS232Manager::send(String msg) {

    if(msg.length() == 0) return;

    String trame = "<ID01><L1><PA><FA><Ma><WC><FA><CA>" + msg + "<E>";

    serial->print(trame);

    Serial.println("RS232 envoyé : " + trame);
}