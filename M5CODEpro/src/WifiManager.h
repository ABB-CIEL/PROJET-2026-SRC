#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <M5Core2.h>
#include <WiFi.h>

class WifiManager {
private:
    String ap_ssid, ap_password;
    String sta_ssid, sta_password;
    String mode; // AP, STA, AP_STA
    bool active;

    IPAddress local_ip, gateway, subnet, dns;

public:
    WifiManager();
    
    void setApCredentials(String ssid, String password);
    void setStaCredentials(String ssid, String password);
    void setMode(String m);
    void setNetwork(IPAddress ip, IPAddress gw, IPAddress mask, IPAddress dnsServer);
    
    bool activate();
    void deactivate();
    bool isActive();
    IPAddress getIP();
    String getMode();
};
#endif