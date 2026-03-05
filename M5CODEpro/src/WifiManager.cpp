#include "WifiManager.h"

WifiManager::WifiManager() {

    active = false;
    mode = "AP";

}

void WifiManager::setCredentials(String s, String p) {

    ssid = s;
    password = p;

}

void WifiManager::setMode(String m) {

    mode = m;

}

void WifiManager::setNetwork(IPAddress ip, IPAddress gw, IPAddress mask, IPAddress dnsServer) {

    local_ip = ip;
    gateway = gw;
    subnet = mask;
    dns = dnsServer;

}

bool WifiManager::activate() {

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    delay(200);

    if (mode == "AP") {

        WiFi.mode(WIFI_AP);

        if (!WiFi.softAP(ssid.c_str(), password.c_str())) {

            Serial.println("Erreur creation AP");
            return false;

        }

        Serial.println("AP actif : " + WiFi.softAPIP().toString());

    }
    else {

        WiFi.mode(WIFI_STA);

        WiFi.config(local_ip, gateway, subnet, dns);

        WiFi.begin(ssid.c_str(), password.c_str());

        unsigned long start = millis();

        while (WiFi.status() != WL_CONNECTED && millis() - start < 12000) {

            delay(500);
            Serial.print(".");

        }

        if (WiFi.status() != WL_CONNECTED) {

            Serial.println("Connexion STA echouee");
            return false;

        }

        Serial.println("Connecte : " + WiFi.localIP().toString());

    }

    active = true;

    return true;
}

void WifiManager::deactivate() {

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    active = false;

}

bool WifiManager::isActive() {

    return active;

}

IPAddress WifiManager::getIP() {

    if (mode == "AP")
        return WiFi.softAPIP();
    else
        return WiFi.localIP();

}

String WifiManager::getMode() {

    return mode;

}