/**
 * @file ConfigManager.cpp
 * @brief Implémentation de la gestion de la configuration (Preferences)
 * @author Étudiant BTS CIEL IR
 * @date 2026
 */

#include "ConfigManager.h"

// ============================================================
// REGION : Chargement et Sauvegarde
// ============================================================

/**
 * @brief Charge les paramètres depuis la mémoire non-volatile
 */
void CConfigManager::load()
{
    this->prefs.begin("wifi", false);
    
    this->ap_ssid = this->prefs.getString("ap_ssid", "M5Journal");
    this->ap_password = this->prefs.getString("ap_pass", "btsciel26");
    this->sta_ssid = this->prefs.getString("sta_ssid", "Nth01");
    this->sta_password = this->prefs.getString("sta_pass", "thierryh1");
    this->udp_port = this->prefs.getInt("port", 1470);
    this->config_port = this->prefs.getInt("cfg_port", 1000);
    this->baud_rate = this->prefs.getInt("baud", 9600);
    
    this->data_bits = this->prefs.getInt("db", 8);
    this->parity = this->prefs.getString("par", "N");
    this->stop_bits = this->prefs.getInt("sb", 1);
    this->serial_type = this->prefs.getString("stype", "RS232");
    
    this->wifi_mode = this->prefs.getString("mode", "AP");
    
    this->ip = this->prefs.getString("ip", "0.0.0.0");
    this->gateway = this->prefs.getString("gw", "0.0.0.0");
    this->subnet = this->prefs.getString("mask", "255.255.255.0");
    this->dns = this->prefs.getString("dns", "8.8.8.8");

    this->prefs.end();
}

/**
 * @brief Enregistre la configuration actuelle
 */
void CConfigManager::save()
{
    this->prefs.begin("wifi", false);

    this->prefs.putString("ap_ssid", this->ap_ssid);
    this->prefs.putString("ap_pass", this->ap_password);
    this->prefs.putString("sta_ssid", this->sta_ssid);
    this->prefs.putString("sta_pass", this->sta_password);
    this->prefs.putInt("port", this->udp_port);
    this->prefs.putInt("cfg_port", this->config_port);
    this->prefs.putInt("baud", this->baud_rate);
    
    this->prefs.putInt("db", this->data_bits);
    this->prefs.putString("par", this->parity);
    this->prefs.putInt("sb", this->stop_bits);
    this->prefs.putString("stype", this->serial_type);

    this->prefs.putString("mode", this->wifi_mode);
    this->prefs.putString("ip", this->ip);
    this->prefs.putString("gw", this->gateway);
    this->prefs.putString("mask", this->subnet);
    this->prefs.putString("dns", this->dns);

    this->prefs.end();
}

// ============================================================
// REGION : Maintenance Système
// ============================================================

/**
 * @brief Réinitialise les paramètres d'usine
 */
void CConfigManager::factoryReset()
{
    this->prefs.begin("wifi", false);
    this->prefs.clear();
    this->prefs.end();

    ESP.restart();
}