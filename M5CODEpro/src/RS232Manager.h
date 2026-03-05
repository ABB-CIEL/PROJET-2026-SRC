#ifndef RS232_MANAGER_H
#define RS232_MANAGER_H

#include <Arduino.h>

class RS232Manager {

private:
    HardwareSerial* serial;
    int baudRate;

public:
    RS232Manager(HardwareSerial& port);

    void begin(int baud);
    void send(String msg);

};

#endif