#include "ConfigManager.h"

void ConfigManager::load() {

    prefs.begin("wifi", false);

    ssid = prefs.getString("ssid", "M5Journal");
    password = prefs.getString("pass", "btsciel26");
    udp_port = prefs.getInt("port", 1470);
    baud_rate = prefs.getInt("baud", 9600);
    wifi_mode = prefs.getString("mode", "AP");

    prefs.end();
}

void ConfigManager::save() {

    prefs.begin("wifi", false);

    prefs.putString("ssid", ssid);
    prefs.putString("pass", password);
    prefs.putInt("port", udp_port);
    prefs.putInt("baud", baud_rate);
    prefs.putString("mode", wifi_mode);

    prefs.end();
}

void ConfigManager::factoryReset() {

    prefs.begin("wifi", false);
    prefs.clear();
    prefs.end();

    ESP.restart();
}