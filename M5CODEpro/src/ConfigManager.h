#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Preferences.h>
#include <Arduino.h>

class ConfigManager {

private:
    Preferences prefs;

public:

    String ssid;
    String password;
    int udp_port;
    int baud_rate;
    String wifi_mode;

    void load();
    void save();
    void factoryReset();

};

#endif