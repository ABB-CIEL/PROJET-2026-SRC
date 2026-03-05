#ifndef UDP_MANAGER_H
#define UDP_MANAGER_H

#include <Arduino.h>
#include <WiFiUdp.h>

class UDPManager {

private:
    WiFiUDP udpJournal;
    WiFiUDP udpConfig;

    int journalPort;
    int configPort;

public:

    UDPManager();

    void begin(int portJournal, int portConfig);

    bool receiveJournal(String &msg);

    bool receiveConfig(String &msg, IPAddress &ip, int &port);

    void sendConfigResponse(IPAddress ip, int port, String message);

    void stop();
};

#endif 
