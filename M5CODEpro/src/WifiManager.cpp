/**
 * @file WifiManager.cpp
 * @brief Implémentation de la gestion de la connectivité WiFi (AP, STA, AP_STA)
 * @author Étudiant BTS CIEL IR
 * @date 2026
 */

#include "WifiManager.h"

// ============================================================
// REGION : Définitions et Constantes
// ============================================================

#if !defined(IP_NAPT_MAX)
    #define IP_NAPT_MAX     16
    #define IP_NAPT_TIMEOUT 300
#endif

// ============================================================
// REGION : Constructeur et Paramétrage
// ============================================================

/** @class CWifiManager
 *  @brief Gère les différents modes WiFi de l'ESP32
 */
CWifiManager::CWifiManager()
{
    this->active = false;
    this->mode = "AP";
}

/**
 * @brief Définit les identifiants du point d'accès
 * @param ssid Nom du réseau
 * @param password Mot de passe
 */
void CWifiManager::setApCredentials(String ssid, String password)
{
    this->ap_ssid = ssid;
    this->ap_password = password;
}

/**
 * @brief Définit les identifiants pour la connexion à une box
 * @param ssid Nom du réseau
 * @param password Mot de passe
 */
void CWifiManager::setStaCredentials(String ssid, String password)
{
    this->sta_ssid = ssid;
    this->sta_password = password;
}

/**
 * @brief Définit le mode de fonctionnement
 * @param m Mode ("AP", "STA", "AP_STA")
 */
void CWifiManager::setMode(String m)
{
    this->mode = m;
}

/**
 * @brief Configure les paramètres IP statiques
 * @param ip Adresse IP locale
 * @param gw Passerelle
 * @param mask Masque de sous-réseau
 * @param dnsServer Serveur DNS
 */
void CWifiManager::setNetwork(IPAddress ip, IPAddress gw, IPAddress mask, IPAddress dnsServer)
{
    this->local_ip = ip;
    this->gateway = gw;
    this->subnet = mask;
    this->dns = dnsServer;
}

// ============================================================
// REGION : Gestion de l'état de connexion
// ============================================================

/**
 * @brief Active le WiFi selon la configuration définie
 * @return true si l'activation est réussie
 */
bool CWifiManager::activate()
{
    WiFi.disconnect(true);
    WiFi.softAPdisconnect(true);
    delay(50);

    if (this->mode == "AP" || this->mode == "AP_STA")
    {
        WiFi.mode(this->mode == "AP_STA" ? WIFI_AP_STA : WIFI_AP);
        delay(100);

        if (this->ap_password.length() > 0 && this->ap_password.length() < 8)
        {
            return false;
        }

        if (this->local_ip != IPAddress(0, 0, 0, 0))
        {
            WiFi.softAPConfig(this->local_ip, this->gateway, this->subnet);
        }

        const char* pass = this->ap_password.length() > 0 ? this->ap_password.c_str() : "";
        if (!WiFi.softAP(this->ap_ssid.c_str(), pass))
        {
            return false;
        }
    }
    
    if (this->mode == "STA" || this->mode == "AP_STA")
    {
        if (this->mode == "STA")
        {
            WiFi.mode(WIFI_STA);
        }

        if (this->sta_ssid.length() == 0)
        {
            return false;
        }

        WiFi.setHostname("M5JournalPRO");
        WiFi.setAutoReconnect(true);

        if (this->local_ip != IPAddress(0, 0, 0, 0))
        {
             WiFi.config(this->local_ip, this->gateway, this->subnet, this->dns);
        }
        else
        {
            WiFi.config(IPAddress(0,0,0,0), IPAddress(0,0,0,0), IPAddress(0,0,0,0));
        }
        
        WiFi.begin(this->sta_ssid.c_str(), this->sta_password.c_str());

        unsigned long start = millis();
        const int CONN_TIMEOUT = 12000;

        while (WiFi.status() != WL_CONNECTED && millis() - start < CONN_TIMEOUT)
        {
            if (WiFi.status() == WL_CONNECT_FAILED)
            {
                break;
            }
            delay(500);
        }

        if (WiFi.status() != WL_CONNECTED)
        {
            return false;
        }
    }

    this->active = true;
    return true;
}

/**
 * @brief Désactive proprement toutes les interfaces WiFi
 */
void CWifiManager::deactivate()
{
    WiFi.disconnect(true);
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    this->active = false;
}

/**
 * @brief Vérifie si le module est actif
 * @return true si actif
 */
bool CWifiManager::isActive()
{
    return this->active;
}

/**
 * @brief Récupère l'adresse IP active
 * @return Adresse IP (SoftAP ou Station)
 */
IPAddress CWifiManager::getIP()
{
    if (this->mode == "AP")
    {
        return WiFi.softAPIP();
    }
    return WiFi.localIP();
}

/**
 * @brief Récupère le mode actuel
 * @return Nom du mode
 */
String CWifiManager::getMode()
{
    return this->mode;
}