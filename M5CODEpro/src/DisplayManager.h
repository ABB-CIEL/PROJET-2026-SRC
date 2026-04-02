#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <M5Core2.h>
#include "ConfigManager.h"
#include "WifiManager.h"

class DisplayManager {
private:
    ConfigManager& config;
    WifiManager& wifiManager;
    int currentTab;

    unsigned long lastActivity; // Pour le screensaver
    bool isDimmed;              // État de la luminosité

    // Témoin d'acquittement RS232
    bool lastAckSuccess;
    unsigned long lastAckTime;

    // Mode Message (Popup défilable)
    bool isMessageMode;
    String messageText;
    int messageScroll;
    int lastTouchY;
    int totalMessageHeight;
    int scanScroll; // Position de défilement liste WiFi

    void clearContent();
    void showWiFi();
    void showSTA();
    void showConfig();
    void showStatus();
    void showReset();
    void showScan();
    void drawScanList(int n); // Helper pour redessiner la liste sans rescanner
    void drawMessageScreen();

    // Helpers UI pour le look "Pro"
    void drawDataRow(String label, String value, int y, uint16_t valueColor = TFT_WHITE);
    void drawSectionTitle(String title, int y);

public:
    DisplayManager(ConfigManager& cfg, WifiManager& wifi);

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
    void showConnecting(const String& message);
};

#endif