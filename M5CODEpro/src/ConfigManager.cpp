#include "ConfigManager.h"

void ConfigManager::load() {

    prefs.begin("wifi", false);
    
    ap_ssid = prefs.getString("ap_ssid", "M5Journal");
    ap_password = prefs.getString("ap_pass", "btsciel26");
    sta_ssid = prefs.getString("sta_ssid", "Nth01");
    sta_password = prefs.getString("sta_pass", "thierryh1");
    udp_port = prefs.getInt("port", 1470);
    baud_rate = prefs.getInt("baud", 9600);
    
    // Nouveaux paramètres RS232
    data_bits = prefs.getInt("db", 8);
    parity = prefs.getString("par", "N"); // N=None, E=Even, O=Odd
    stop_bits = prefs.getInt("sb", 1);
    serial_type = prefs.getString("stype", "RS232"); // RS232, TTL...
    
    wifi_mode = prefs.getString("mode", "AP");
    
    // Par défaut : DHCP (0.0.0.0) pour être prêt à l'emploi
    ip = prefs.getString("ip", "0.0.0.0");
    gateway = prefs.getString("gw", "0.0.0.0");
    subnet = prefs.getString("mask", "255.255.255.0");
    dns = prefs.getString("dns", "8.8.8.8");

    prefs.end();
}

void ConfigManager::save() {

    prefs.begin("wifi", false);

    prefs.putString("ap_ssid", ap_ssid);
    prefs.putString("ap_pass", ap_password);
    prefs.putString("sta_ssid", sta_ssid);
    prefs.putString("sta_pass", sta_password);
    prefs.putInt("port", udp_port);
    prefs.putInt("baud", baud_rate);
    
    prefs.putInt("db", data_bits);
    prefs.putString("par", parity);
    prefs.putInt("sb", stop_bits);
    prefs.putString("stype", serial_type);

    prefs.putString("mode", wifi_mode);
    prefs.putString("ip", ip);
    prefs.putString("gw", gateway);
    prefs.putString("mask", subnet);
    prefs.putString("dns", dns);

    prefs.end();
}

void ConfigManager::factoryReset() {

    prefs.begin("wifi", false);
    prefs.clear();
    prefs.end();

    ESP.restart();
}