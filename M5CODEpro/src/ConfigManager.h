#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Preferences.h>
#include <Arduino.h>

class ConfigManager {

private:
    Preferences prefs;

public:

    String ap_ssid;
    String ap_password;
    String sta_ssid;
    String sta_password;
    int udp_port;
    int config_port;
    int baud_rate;
    
    // Paramètres RS232 ajoutés
    int data_bits;
    String parity;
    int stop_bits;
    String serial_type;

    String wifi_mode;
    String ip;
    String gateway;
    String subnet;
    String dns;

    void load();
    void save();
    void factoryReset();

};

#endif