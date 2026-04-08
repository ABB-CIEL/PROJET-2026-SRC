#include "WifiManager.h"

// --- Ajout pour le NAT (Partage de connexion) ---
#if !defined(IP_NAPT_MAX)
#define IP_NAPT_MAX     16
#define IP_NAPT_TIMEOUT 300
#endif

// extern "C" {
//     void ip_napt_init(uint16_t max_nat, uint16_t nat_timeout);
//     void ip_napt_enable(uint32_t addr, int enable);
// }
// ------------------------------------------------

WifiManager::WifiManager() {

    active = false;
    mode = "AP";

}

void WifiManager::setApCredentials(String ssid, String password) {
    this->ap_ssid = ssid;
    this->ap_password = password;
}

void WifiManager::setStaCredentials(String ssid, String password) {
    this->sta_ssid = ssid;
    this->sta_password = password;
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

    // On s'assure que le mode précédent est bien coupé proprement
    // NE PAS utiliser WIFI_OFF ici, cela éteint la radio et peut bloquer le redémarrage immédiat en AP+STA
    WiFi.disconnect(true);
    WiFi.softAPdisconnect(true);
    delay(50);

    // Gestion du mode HYBRIDE (AP+STA) ou AP seul
    if (mode == "AP" || mode == "AP_STA") {

        // Active le mode approprié
        WiFi.mode(mode == "AP_STA" ? WIFI_AP_STA : WIFI_AP);
        delay(100); // Laisse le temps au matériel de s'initialiser

        // Validation du password pour AP (8-63 caractères ou vide)
        if (ap_password.length() > 0 && ap_password.length() < 8) {
            return false;
        }

        // Configuration explicite du SoftAP : SSID, Pass, Channel 1, Hidden 0, MaxConn 4
        // CORRECTION : On ne force plus le channel 1. En mode AP_STA, l'AP doit suivre le canal du STA.
        const char* pass = ap_password.length() > 0 ? ap_password.c_str() : "";
        // Utilisation des paramètres par défaut (channel auto/1) pour éviter le conflit radio
        if (!WiFi.softAP(ap_ssid.c_str(), pass)) {
            return false;
        }
    }
    
    // Si on est en STA ou en AP_STA, on lance la connexion client
    if (mode == "STA" || mode == "AP_STA") {
        
        // Si AP_STA, le mode est déjà défini au dessus, sinon on le force
        if (mode == "STA") WiFi.mode(WIFI_STA);

        // Si le SSID est vide, inutile d'essayer de se connecter (évite le blocage)
        if (sta_ssid.length() == 0) return false;

        WiFi.setHostname("M5JournalPRO");
        WiFi.setAutoReconnect(true);

        // Configuration IP statique uniquement si une IP valide est fournie (et différente de 0.0.0.0)
        if (local_ip != IPAddress(0, 0, 0, 0)) {
             WiFi.config(local_ip, gateway, subnet, dns);
        }
        else {
            // S'assurer que le DHCP est actif si aucune IP statique n'est définie
            // Important : On passe des 0 pour dire "Oublie l'IP statique précédente"
            WiFi.config(IPAddress(0,0,0,0), IPAddress(0,0,0,0), IPAddress(0,0,0,0));
        }
        
        WiFi.begin(sta_ssid.c_str(), sta_password.c_str());

        unsigned long start = millis();

        while (WiFi.status() != WL_CONNECTED && millis() - start < 12000) {
            // Si la connexion échoue explicitement (ex: mauvais mot de passe), on arrête plus tôt
            if (WiFi.status() == WL_CONNECT_FAILED) break;
            delay(500);
        }

        // En mode Hybride, on ne retourne faux que si le STA échoue vraiment ? 
        // Pour l'instant on considère que ça doit marcher.
        if (WiFi.status() != WL_CONNECTED) {
            return false;
        }

    }

    // Activation du NAT (Routeur) si on est en mode Hybride (AP + STA)
    // Permet aux clients du M5 d'accéder à Internet via le STA
    if ((mode == "AP_STA") && (WiFi.status() == WL_CONNECTED)) {
        // ip_napt_init(IP_NAPT_MAX, IP_NAPT_TIMEOUT);
        // ip_napt_enable(WiFi.softAPIP(), 1);
        // Note : Le patch NAPT doit être actif dans le framework ESP32 utilisé
    }

    active = true;

    return true;
}

void WifiManager::deactivate() {

    WiFi.disconnect(true);
    WiFi.softAPdisconnect(true); // Force la coupure du point d'accès
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