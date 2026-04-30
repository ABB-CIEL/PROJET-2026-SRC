/**
 * @file ConfigManager.h
 * @brief Définition de la classe de gestion de la configuration en EEPROM
 * @author Étudiant BTS CIEL IR
 * @date 2026
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Preferences.h>
#include <Arduino.h>

/** @class CConfigManager
 *  @brief Gère le stockage et la récupération des paramètres système via Preferences
 */
class CConfigManager
{
private:
    Preferences prefs; ///< Instance pour l'accès à la mémoire non-volatile

public:
    String ap_ssid;     ///< SSID du point d'accès
    String ap_password; ///< Mot de passe du point d'accès
    String sta_ssid;    ///< SSID de la station à rejoindre
    String sta_password;///< Mot de passe de la station
    int udp_port;       ///< Port UDP pour le journal
    int config_port;    ///< Port UDP pour la configuration
    int baud_rate;      ///< Vitesse du port série
    
    int data_bits;      ///< Nombre de bits de données (7 ou 8)
    String parity;      ///< Parité ("N", "E", "O")
    int stop_bits;      ///< Nombre de bits de stop (1 ou 2)
    String serial_type; ///< Type de liaison série (RS232, TTL)

    String wifi_mode;   ///< Mode WiFi ("AP", "STA", "AP_STA")
    String ip;          ///< Adresse IP statique (0.0.0.0 si DHCP)
    String gateway;     ///< Passerelle
    String subnet;      ///< Masque de sous-réseau
    String dns;         ///< Serveur DNS

    /**
     * @brief Charge la configuration depuis la mémoire Preferences
     */
    void load();

    /**
     * @brief Sauvegarde la configuration actuelle en mémoire
     */
    void save();

    /**
     * @brief Réinitialise les paramètres par défaut
     */
    void factoryReset();
};

#endif