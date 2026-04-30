/**
 * @file DisplayManager.h
 * @brief Définition de la classe de gestion de l'interface graphique M5Stack
 * @author Étudiant BTS CIEL IR
 * @date 2026
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <M5Core2.h>
#include "ConfigManager.h"
#include "WifiManager.h"

/** @class CDisplayManager
 *  @brief Gère l'affichage LCD et les interactions tactiles sur M5Stack Core2
 */
class CDisplayManager
{
private:
    CConfigManager& config;   ///< Référence à la configuration
    CWifiManager& wifiManager; ///< Référence au gestionnaire WiFi
    int currentTab;          ///< Index de l'onglet actif

    unsigned long lastActivity; ///< Horodatage de la dernière action (screensaver)
    bool isDimmed;              ///< État réduit de la luminosité

    bool lastAckSuccess;        ///< Résultat du dernier acquittement RS232
    unsigned long lastAckTime;  ///< Temps du dernier acquittement

    bool isMessageMode;         ///< Indique si une popup de message est active
    String messageText;         ///< Contenu du message affiché
    int messageScroll;          ///< Position verticale de défilement du message
    int lastTouchY;             ///< Dernière position Y du toucher
    int totalMessageHeight;     ///< Hauteur calculée du message complet
    int scanScroll;             ///< Position de défilement de la liste WiFi

    /**
     * @brief Efface la zone centrale de l'écran
     */
    void clearContent();

    /**
     * @brief Affiche l'onglet de statut WiFi global
     */
    void showWiFi();

    /**
     * @brief Affiche les détails de la connexion station
     */
    void showSTA();

    /**
     * @brief Affiche l'écran de configuration des paramètres
     */
    void showConfig();

    /**
     * @brief Affiche les informations système et état batterie
     */
    void showStatus();

    /**
     * @brief Affiche l'écran d'avertissement de réinitialisation
     */
    void showReset();

    /**
     * @brief Lance un scan et affiche les réseaux disponibles
     */
    void showScan();

    /**
     * @brief Dessine la liste des réseaux scannés
     * @param n Nombre de réseaux trouvés
     */
    void drawScanList(int n);

    /**
     * @brief Dessine l'écran de message défilant
     */
    void drawMessageScreen();

    /**
     * @brief Dessine une ligne de données avec label et valeur
     */
    void drawDataRow(String label, String value, int y, uint16_t valueColor = TFT_WHITE);

    /**
     * @brief Dessine un titre de section stylisé
     */
    void drawSectionTitle(String title, int y);

public:
    CDisplayManager(CConfigManager& cfg, CWifiManager& wifi);

    void showSplashScreen();
    void fadeTransition();
    void drawHeader();
    void drawTabs();
    void refreshUI();
    void setCurrentTab(int tab);
    int getCurrentTab();
    int update();
    void showReceivedMessage(const String& msg);
    void setAckStatus(bool success);

    /**
     * @brief Affiche un écran d'attente de connexion
     * @param message Message d'attente
     */
    void showConnecting(const String& message);
};

#endif