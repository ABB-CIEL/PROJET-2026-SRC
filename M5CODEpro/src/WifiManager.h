#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

class WifiManager {

private:

    String ssid;
    String password;
    String mode;

    IPAddress local_ip;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns;

    bool active;

public:

    WifiManager();

    void setCredentials(String s, String p);

    void setMode(String m);

    void setNetwork(IPAddress ip, IPAddress gw, IPAddress mask, IPAddress dnsServer);

    bool activate();

    void deactivate();

    bool isActive();

    IPAddress getIP();

    String getMode();

};

#endif