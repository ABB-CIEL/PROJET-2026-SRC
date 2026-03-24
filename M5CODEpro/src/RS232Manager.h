#ifndef RS232MANAGER_H
#define RS232MANAGER_H

#include <Arduino.h>
#include <HardwareSerial.h>

class RS232Manager {
private:
    HardwareSerial* serial;
    String ID;
    String payloadBase;
    int baudRate;

    String calculateLRC(String data);
    String sendTrame(String msg);

public:
    RS232Manager(HardwareSerial& port);
    void begin(int baud);
    
    // Convertit les params (8, "N", 1) en constante Arduino (ex: SERIAL_8N1)
    uint32_t convertConfig(int dataBits, String parity, int stopBits);

    String send(String msg);
    String receive();
    
    // Commandes spécifiques
    void sendAnimation(int animIndex, String customText = "");
    void sendCustomText(String text, int animIndex);
};

#endif