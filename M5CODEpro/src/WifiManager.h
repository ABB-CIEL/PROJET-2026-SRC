/**
 * @file WifiManager.h
 * @brief Définition de la classe de gestion WiFi
 * @author Étudiant BTS CIEL IR
 * @date 2026
 */

#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <M5Core2.h>
#include <WiFi.h>

/** @class CWifiManager
 *  @brief Gère la connectivité sans fil (Point d'accès et Station)
 */
class CWifiManager
{
private:
    String ap_ssid;     ///< SSID du point d'accès (Note: format snake_case conservé pour JSON)
    String ap_password; ///< Mot de passe du point d'accès
    String sta_ssid;    ///< SSID de la box cible
    String sta_password;///< Mot de passe de la box cible
    String mode;        ///< Mode actuel (AP, STA, AP_STA)
    bool active;        ///< État du module

    IPAddress local_ip; ///< IP statique
    IPAddress gateway;  ///< Passerelle
    IPAddress subnet;   ///< Masque
    IPAddress dns;      ///< Serveur DNS

public:
    CWifiManager();
    
    /**
     * @brief Configure les identifiants AP
     */
    void setApCredentials(String ssid, String password);

    /**
     * @brief Configure les identifiants Station
     */
    void setStaCredentials(String ssid, String password);

    /**
     * @brief Définit le mode WiFi
     */
    void setMode(String m);

    /**
     * @brief Configure les paramètres IP statiques
     */
    void setNetwork(IPAddress ip, IPAddress gw, IPAddress mask, IPAddress dnsServer);
    
    /**
     * @brief Active le WiFi
     * @return true si succès
     */
    bool activate();

    /**
     * @brief Arrête le WiFi
     */
    void deactivate();

    /**
     * @brief État d'activation
     */
    bool isActive();

    /**
     * @brief Récupère l'IP
     */
    IPAddress getIP();

    /**
     * @brief Récupère le mode
     */
    String getMode();
};
#endif