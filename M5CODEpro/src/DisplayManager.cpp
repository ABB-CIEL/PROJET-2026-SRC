#include "DisplayManager.h"
#include <time.h>

// --- CONSTANTES THEME ---
// Couleurs du thème "Pro" (Cyberpunk / Industriel propre)
#define THEME_BG        TFT_BLACK
#define THEME_ACCENT    TFT_CYAN
#define THEME_TEXT      TFT_WHITE
#define THEME_LABEL     TFT_LIGHTGREY
#define THEME_BOX       0x18E3  // Gris foncé bleuté
#define THEME_ROW       0x0841  // Gris très sombre pour alternance

// ==========================================
// === INITIALISATION & SPLASH SCREEN ===
// ==========================================
DisplayManager::DisplayManager(ConfigManager& cfg, WifiManager& wifi) : config(cfg), wifiManager(wifi) {
    currentTab = 0; // Onglet par défaut
    lastActivity = millis();
    isDimmed = false;
    M5.Lcd.setBrightness(128); // Luminosité par défaut (0-255)
    isMessageMode = false;
    messageScroll = 0;
    lastTouchY = -1;
    totalMessageHeight = 0;
    scanScroll = 0;
    lastAckSuccess = false;
    lastAckTime = 0;
}

void DisplayManager::showSplashScreen() {
    M5.Lcd.fillScreen(THEME_BG);
    M5.Lcd.setTextColor(THEME_ACCENT);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("M5 CODE PRO", 160, 100, 4);
    M5.Lcd.setTextColor(TFT_LIGHTGREY);
    M5.Lcd.drawString("v1.1 - Initialisation...", 160, 130, 2);
    M5.Lcd.setTextDatum(TL_DATUM);

    // Barre de chargement
    for (int i = 0; i < 200; i+=4) {
        M5.Lcd.fillRect(60, 160, i, 10, THEME_ACCENT);
        delay(10);
    }
    delay(300);
}

void DisplayManager::fadeTransition() {
    // Transition simple : effacement rapide
    // Animation "Wipe" High-Tech (Balayage Cyan rapide)
    // On passe une barre verticale cyan qui efface l'ancien écran et laisse du noir derrière
    int step = 32; // Largeur de la barre de balayage
    
    for (int x = 0; x <= 320; x += step) {
        // Dessine la barre de balayage (front d'effacement)
        if (x < 320) M5.Lcd.fillRect(x, 30, step, 185, THEME_ACCENT);
        
        // Efface la traînée en noir (couleur de fond)
        if (x >= step) M5.Lcd.fillRect(x - step, 30, step, 185, THEME_BG);
        
        delay(15); // Vitesse de l'animation
    }
    // Sécurité : s'assure que tout est bien noir à la fin
    M5.Lcd.fillRect(0, 30, 320, 185, THEME_BG);
}

// ==========================================
// === ELEMENTS GLOBAUX (HEADER/TABS) ===
// ==========================================
void DisplayManager::drawHeader() {
    // Header plat et propre
    M5.Lcd.fillRect(0, 0, 320, 30, 0x1082); // Gris sombre
    M5.Lcd.drawFastHLine(0, 29, 320, THEME_ACCENT);

    // Batterie
    float vbat = M5.Axp.GetBatVoltage();
    int percent = map(vbat * 100, 330, 420, 0, 100);
    percent = constrain(percent, 0, 100);
    uint16_t batColor = percent > 50 ? TFT_GREEN : (percent > 20 ? TFT_YELLOW : TFT_RED);
    
    int batX = 5;
    M5.Lcd.drawRect(batX, 9, 28, 12, TFT_WHITE);
    M5.Lcd.fillRect(batX + 2, 11, (percent * 24) / 100, 8, batColor);
    M5.Lcd.fillRect(batX + 28, 11, 2, 8, TFT_WHITE); // Plot batterie
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_WHITE, 0x1082);
    M5.Lcd.setCursor(batX + 36, 11);
    M5.Lcd.printf("%d%%", percent);

    // --- Témoin ACK RS232 (Cercle lumineux temporaire) ---
    if (lastAckTime > 0 && millis() - lastAckTime < 3000) {
        uint16_t ackColor = lastAckSuccess ? TFT_GREEN : TFT_RED;
        M5.Lcd.fillCircle(110, 15, 4, ackColor);
        M5.Lcd.drawCircle(110, 15, 5, TFT_WHITE);
    }

    // --- Icône WiFi Dynamique ---
    // Zone de nettoyage pour l'icône (avant l'heure)
    M5.Lcd.fillRect(225, 5, 25, 20, 0x1082); 

    if (wifiManager.isActive()) {
        int bars = 0;
        uint16_t signalColor = TFT_DARKGREY;

        if (config.wifi_mode == "AP") {
            bars = 4; // AP est toujours à fond
            signalColor = TFT_BLUE; // Bleu pour le mode AP
        } else {
            int32_t rssi = WiFi.RSSI();
            if (rssi > -50) bars = 4;
            else if (rssi > -70) bars = 3;
            else if (rssi > -80) bars = 2;
            else if (rssi > -90) bars = 1;
            
            signalColor = (bars >= 3) ? TFT_GREEN : (bars >= 2) ? TFT_YELLOW : TFT_RED;
        }
        
        // En mode Hybride, on met une couleur spécifique (Magenta) si connecté
        if (config.wifi_mode == "AP_STA") signalColor = TFT_MAGENTA;

        // Dessin des barres
        for (int b = 0; b < 4; b++) {
            int h = (b + 1) * 3 + 2; // Hauteur progressive
            if (b < bars) M5.Lcd.fillRect(228 + (b * 5), 22 - h, 3, h, signalColor);
            else M5.Lcd.drawRect(228 + (b * 5), 22 - h, 3, h, TFT_DARKGREY);
        }
    } else {
        // Croix rouge si inactif
        M5.Lcd.drawLine(230, 20, 240, 10, TFT_RED);
        M5.Lcd.drawLine(230, 10, 240, 20, TFT_RED);
    }

    // Titre Centré
    M5.Lcd.setTextDatum(MC_DATUM); // Middle Center
    M5.Lcd.drawString("M5 CODE PRO", 160, 15, 2);
    M5.Lcd.setTextDatum(TL_DATUM); // Reset Top Left

    // Heure
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    char timeStr[9];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    
    // Efface la zone de l'heure pour éviter la superposition
    M5.Lcd.fillRect(255, 0, 65, 29, 0x1082);
    
    M5.Lcd.setTextColor(THEME_ACCENT);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(MR_DATUM); // Middle Right
    M5.Lcd.drawString(timeStr, 315, 15, 2);
    M5.Lcd.setTextDatum(TL_DATUM);
}

void DisplayManager::drawTabs() {
    const char* names[] = {"WiFi", "Config", "Statut", "Scan", "STA", "Reset"};
    
    // Barre de fond des onglets
    M5.Lcd.fillRect(0, 215, 320, 25, 0x2104); 
    M5.Lcd.drawFastHLine(0, 215, 320, TFT_DARKGREY);

    // 6 onglets sur 320px = ~53px par onglet
    int tabWidth = 320 / 6; 

    for (int i = 0; i < 6; i++) {
        bool active = (currentTab == i);
        int x = i * tabWidth;
        
        // Fond de l'onglet
        if (active) {
            // L'onglet actif devient noir (THEME_BG) pour fusionner avec le contenu
            // Cela crée un effet de "Dossier ouvert" continu
            M5.Lcd.fillRect(x, 215, tabWidth, 25, THEME_BG); 
            M5.Lcd.drawFastHLine(x, 215, tabWidth, THEME_ACCENT); // Ligne de surbrillance
        }
        
        // Séparateur vertical
        if (i < 5 && !active && (currentTab != i + 1)) M5.Lcd.drawFastVLine(x + tabWidth - 1, 218, 19, TFT_DARKGREY);

        // Texte
        M5.Lcd.setTextColor(active ? THEME_ACCENT : TFT_LIGHTGREY);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextDatum(MC_DATUM);
        M5.Lcd.drawString(names[i], x + (tabWidth/2), 227);
    }
    M5.Lcd.setTextDatum(TL_DATUM); // Reset standard
}

// ==========================================
// === HELPERS GRAPHIQUES ===
// ==========================================
void DisplayManager::drawSectionTitle(String title, int y) {
    M5.Lcd.setTextColor(THEME_ACCENT, THEME_BG);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString(title, 15, y, 2); // Légère indentation
    M5.Lcd.drawFastHLine(10, y + 20, 300, 0x3186);
}

void DisplayManager::drawDataRow(String label, String value, int y, uint16_t valueColor) {
    // Style "Carte" pro avec fond arrondi et cadre
    int h = 24;      // Hauteur de ligne
    int w = 300;     // Largeur
    int x = 10;      // Marge gauche
    
    // Fond de la ligne (Cadre sombre)
    M5.Lcd.fillRoundRect(x, y - (h/2), w, h, 4, 0x1924); // 0x1924 = Gris ardoise sombre

    M5.Lcd.setTextColor(TFT_LIGHTGREY, 0x1924);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(ML_DATUM);
    M5.Lcd.drawString(label, x + 10, y, 2); 
    
    M5.Lcd.setTextColor(valueColor, 0x1924);
    M5.Lcd.setTextDatum(MR_DATUM); // Alignement à droite

    // Tronquer les valeurs longues pour éviter le débordement
    if (value.length() > 22) {
        value = value.substring(0, 21) + ".";
    }
    M5.Lcd.drawString(value, x + w - 10, y, 2);
    M5.Lcd.setTextDatum(TL_DATUM); // Reset
}

void DisplayManager::clearContent() {
    M5.Lcd.fillRect(0, 30, 320, 185, THEME_BG);
}

// ==========================================
// === PAGES (ONGLETS) ===
// ==========================================
void DisplayManager::showWiFi() {
    clearContent();
    
    // --- HEADER STATUS ---
    uint16_t statusColor = wifiManager.isActive() ? TFT_GREEN : TFT_RED;
    M5.Lcd.fillRoundRect(10, 35, 300, 32, 4, 0x18E3);
    
    // Animation Pulse
    if (wifiManager.isActive()) {
        M5.Lcd.fillCircle(30, 51, 6, TFT_GREEN);
    } else {
        M5.Lcd.fillCircle(30, 51, 6, TFT_RED);
    }
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_WHITE);
    
    // Titre adaptatif selon le mode
    String title = "WIFI INACTIF";
    if (config.wifi_mode == "AP") title = "MODE: ACCESS POINT (AP)";
    else if (config.wifi_mode == "STA") title = "MODE: STATION (CLIENT)";
    else if (config.wifi_mode == "AP_STA") title = "MODE: HYBRIDE (AP + STA)";
    
    M5.Lcd.drawString(title, 50, 44, 2);
    
    // --- AFFICHAGE SELON LE MODE ---
    int y = 100;
    int step = 26;

    if (config.wifi_mode == "STA") {
        // --- MODE STA (CLIENT) SEULEMENT ---
        drawSectionTitle("CLIENT WIFI (INTERNET)", 75);
        drawDataRow("Reseau Cible", config.sta_ssid, y); y += step;
        drawDataRow("IP Obtenue", wifiManager.isActive() ? WiFi.localIP().toString() : "...", y); y += step;
        drawDataRow("Signal (RSSI)", String(WiFi.RSSI()) + " dBm", y); y += step;
        drawDataRow("Passerelle", wifiManager.isActive() ? WiFi.gatewayIP().toString() : "---", y);
    
    } 
    else if (config.wifi_mode == "AP") {
        // --- MODE AP SEULEMENT ---
        drawSectionTitle("POINT D'ACCES (LOCAL)", 75);
        drawDataRow("Mon SSID", config.ap_ssid, y); y += step;
        drawDataRow("Mon IP", wifiManager.isActive() ? WiFi.softAPIP().toString() : "...", y); y += step;
        drawDataRow("Clients", String(WiFi.softAPgetStationNum()), y); y += step;
        drawDataRow("Port UDP", String(config.udp_port), y);
    
    } 
    else if (config.wifi_mode == "AP_STA") {
        // --- MODE HYBRIDE (PROPRE ET SEPARE) ---
        
        // BLOC 1 : HOTSPOT (Haut)
        M5.Lcd.setTextColor(THEME_ACCENT);
        M5.Lcd.drawString("1. HOTSPOT (M5 -> PC)", 15, 75, 2);
        M5.Lcd.drawFastHLine(10, 92, 300, 0x3186);
        
        y = 105;
        // Lignes plus fines pour gagner de la place
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(TFT_LIGHTGREY); M5.Lcd.drawString("SSID:", 20, y, 2);
        M5.Lcd.setTextColor(TFT_WHITE);     M5.Lcd.drawString(config.ap_ssid, 80, y, 2);
        M5.Lcd.setTextColor(TFT_LIGHTGREY); M5.Lcd.drawString("IP:", 190, y, 2);
        M5.Lcd.setTextColor(THEME_ACCENT);  M5.Lcd.drawString(WiFi.softAPIP().toString(), 220, y, 2);
        
        // BLOC 2 : CLIENT (Bas)
        int y2 = 135;
        M5.Lcd.setTextColor(THEME_ACCENT);
        M5.Lcd.drawString("2. CLIENT (M5 -> BOX)", 15, y2, 2);
        M5.Lcd.drawFastHLine(10, y2 + 17, 300, 0x3186);
        
        y2 = 162;
        M5.Lcd.setTextColor(TFT_LIGHTGREY); M5.Lcd.drawString("Cible:", 20, y2, 2);
        M5.Lcd.setTextColor(TFT_WHITE);     M5.Lcd.drawString(config.sta_ssid.substring(0,12), 80, y2, 2); // Tronqué
        M5.Lcd.setTextColor(TFT_LIGHTGREY); M5.Lcd.drawString("IP:", 190, y2, 2);
        M5.Lcd.setTextColor(TFT_GREEN);     M5.Lcd.drawString(WiFi.localIP().toString(), 220, y2, 2);
    }
    
    // Footer action
    M5.Lcd.fillRect(0, 195, 320, 20, 0x1082);
    M5.Lcd.setTextColor(THEME_ACCENT);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("Btn A: ON / OFF", 160, 205);
    M5.Lcd.setTextDatum(TL_DATUM);
}

void DisplayManager::showSTA() {
    clearContent();

    // Status Box en haut
    uint16_t statusColor = wifiManager.isActive() ? TFT_GREEN : TFT_RED;
    M5.Lcd.fillRoundRect(10, 35, 300, 32, 4, 0x18E3);
    M5.Lcd.fillCircle(30, 51, 5, statusColor);
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_WHITE);
    String title = (config.wifi_mode == "AP_STA") ? "MODE: HYBRIDE (STA + AP)" : "MODE: STATION (CLIENT)";
    M5.Lcd.drawString(title, 50, 44, 2);
    
    // Informations détaillées
    drawSectionTitle("CONNEXION", 72);

    int y = 106;
    int step = 25;

    drawDataRow("Reseau Cible", config.sta_ssid, y); y += step;
    
    // WiFi.localIP() est correct pour STA
    drawDataRow("IP Obtenue", wifiManager.isActive() ? WiFi.localIP().toString() : "...", y); y += step;
    
    drawDataRow("Passerelle", wifiManager.isActive() ? WiFi.gatewayIP().toString() : "---", y); y += step;
    
    drawDataRow("Signal (RSSI)", String(WiFi.RSSI()) + " dBm", y); // Maintenant visible !

    // Footer action
    M5.Lcd.fillRect(0, 195, 320, 20, 0x1082);
    M5.Lcd.setTextColor(THEME_ACCENT);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("Btn A: Connecter / Deconnecter", 160, 205);
    M5.Lcd.setTextDatum(TL_DATUM);
}

void DisplayManager::showConfig() {
    clearContent();
    
    
    // Bouton de changement de mode
    M5.Lcd.fillRoundRect(10, 60, 300, 35, 6, 0x18E3);
    M5.Lcd.drawRoundRect(10, 60, 300, 35, 6, THEME_ACCENT);
    M5.Lcd.setTextColor(THEME_ACCENT);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("MODE: " + config.wifi_mode + " (CHANGER)", 160, 78, 2);
    M5.Lcd.setTextDatum(TL_DATUM);

    int y = 115;
    int step = 27;
    drawDataRow("UDP Port", String(config.udp_port), y); y += step;
    drawDataRow("Vitesse RS232", String(config.baud_rate) + " bauds", y); y += step;
    drawDataRow("Config Serie", String(config.data_bits) + config.parity + String(config.stop_bits), y);
    
    M5.Lcd.fillRect(10, 180, 300, 25, 0x1082);
    M5.Lcd.setTextColor(THEME_ACCENT);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("Config via USB/UDP JSON", 160, 192);
    M5.Lcd.setTextDatum(TL_DATUM);
}

void DisplayManager::showStatus() {
    clearContent();
    
    // Retour à un affichage LISTE propre et aéré (Moins surchargé que les blocs)
    drawSectionTitle("DASHBOARD SYSTEME", 40);

    int y = 70;
    int step = 28; // Grand espacement pour la lisibilité
    
    // Uptime
    int uptime = millis() / 1000;
    char upStr[20];
    sprintf(upStr, "%02dh %02dm %02ds", uptime / 3600, (uptime % 3600) / 60, uptime % 60);
    drawDataRow("Uptime", String(upStr), y, TFT_CYAN); y += step;
    
    // RAM
    String ram = String(ESP.getFreeHeap() / 1024) + " KB";
    drawDataRow("RAM Libre", ram, y, TFT_GREEN); y += step;

    // Batterie
    float vbat = M5.Axp.GetBatVoltage();
    int percent = map(vbat * 100, 330, 420, 0, 100);
    String batStr = String(vbat, 2) + "V (" + String(percent) + "%)";
    drawDataRow("Batterie", batStr, y, TFT_YELLOW); y += step;
    
    // MAC
    String mac = WiFi.macAddress();
    drawDataRow("MAC Addr", mac, y, TFT_MAGENTA);
    
    // Footer Warning
    M5.Lcd.setTextColor(TFT_RED, THEME_BG);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("Maintenir Btn C pour RESET", 160, 190);
    M5.Lcd.setTextDatum(TL_DATUM);
}

void DisplayManager::showReset() {
    clearContent();
    
    M5.Lcd.fillRoundRect(40, 60, 240, 100, 10, TFT_RED);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    
    M5.Lcd.drawString("ATTENTION", 160, 85, 4);
    M5.Lcd.drawString("RETOUR USINE", 160, 120, 2);
    M5.Lcd.drawString("Maintenir Bouton C > 3s", 160, 145);
    
    M5.Lcd.setTextDatum(TL_DATUM);
}

void DisplayManager::showScan() {
    clearContent();
    
    // Titres colonnes - Header Pro (Style Card)
    M5.Lcd.fillRoundRect(5, 35, 310, 24, 4, 0x1924);
    
    M5.Lcd.setTextColor(THEME_ACCENT);
    M5.Lcd.setTextSize(1);
    
    M5.Lcd.setTextDatum(ML_DATUM);
    M5.Lcd.drawString("SSID", 15, 47, 2);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("CH", 180, 47, 2);
    M5.Lcd.drawString("SIGNAL", 230, 47, 2);
    M5.Lcd.drawString("SEC", 290, 47, 2);
    M5.Lcd.setTextDatum(TL_DATUM);

    // Message d'attente
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextColor(TFT_LIGHTGREY);
    M5.Lcd.drawString("Scan des reseaux...", 160, 120, 2);
    M5.Lcd.setTextDatum(TL_DATUM);

    // Scan incluant les réseaux cachés
    int n = WiFi.scanNetworks(false, true);
    scanScroll = 0; // Reset du scroll
    drawScanList(n);
    
    // Bouton rescan (Pill Button)
    M5.Lcd.fillRoundRect(60, 192, 200, 22, 11, 0x18E3);
    M5.Lcd.drawRoundRect(60, 192, 200, 22, 11, THEME_ACCENT);
    
    M5.Lcd.setTextColor(THEME_ACCENT);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("ACTUALISER", 160, 204, 2);
    M5.Lcd.setTextDatum(TL_DATUM);
}

void DisplayManager::drawScanList(int n) {
    // Nettoyage zone liste uniquement
    M5.Lcd.fillRect(0, 60, 320, 130, THEME_BG);
    
    if (n == 0) {
        M5.Lcd.setTextColor(TFT_ORANGE);
        M5.Lcd.setTextDatum(MC_DATUM);
        M5.Lcd.drawString("Aucun reseau trouve", 160, 110, 2);
        M5.Lcd.setTextDatum(TL_DATUM);
    } else {
        int y = 62;
        int rowH = 25; // Hauteur de ligne
        
        // Gestion Scrollbar
        if (n > 5) {
            int sbTotalH = 125;
            int sbH = constrain(sbTotalH * 5 / n, 10, sbTotalH);
            int sbY = 62 + (scanScroll * (sbTotalH - sbH) / (n - 5));
            M5.Lcd.fillRoundRect(315, 62, 3, sbTotalH, 1, 0x1082); // Rail
            M5.Lcd.fillRoundRect(315, sbY, 3, sbH, 1, THEME_ACCENT); // Curseur
        }

        for (int i = scanScroll; i < n && i < scanScroll + 5; ++i) { 
            String ssid = WiFi.SSID(i);
            if (ssid.length() > 16) ssid = ssid.substring(0, 15) + ".";
            
            // Ligne alternée
            if (i % 2 == 0) M5.Lcd.fillRect(0, y, 314, rowH, THEME_ROW);
            
            M5.Lcd.setTextColor(TFT_WHITE);
            M5.Lcd.setTextDatum(ML_DATUM);
            M5.Lcd.drawString(ssid, 5, y + (rowH/2), 2);
            
            // Canal
            M5.Lcd.setTextDatum(MC_DATUM);
            M5.Lcd.setTextColor(TFT_LIGHTGREY);
            M5.Lcd.drawString(String(WiFi.channel(i)), 180, y + (rowH/2), 2);

            // Signal Graphique (Barres)
            int32_t rssi = WiFi.RSSI(i);
            int bars = 0;
            if (rssi > -55) bars = 4;
            else if (rssi > -70) bars = 3;
            else if (rssi > -85) bars = 2;
            else bars = 1;
            
            // Dessin des 4 petites barres
            int bX = 215;
            for(int b=0; b<4; b++) {
                uint16_t bCol = (b < bars) ? (bars>2?TFT_GREEN:TFT_YELLOW) : 0x18E3;
                if (bars < 2) bCol = (b < bars) ? TFT_RED : 0x18E3;
                M5.Lcd.fillRect(bX + (b*6), y + 18 - ((b+1)*3), 4, (b+1)*3, bCol);
            }
            
            // Badge Sécurité
            uint16_t secColor = TFT_RED;
            String secText = "KEY";
            wifi_auth_mode_t enc = WiFi.encryptionType(i);
            
            if (enc == WIFI_AUTH_OPEN) { secColor = TFT_GREEN; secText = "OPN"; }
            else if (enc == WIFI_AUTH_WEP) { secColor = TFT_ORANGE; secText = "WEP"; }
            else if (enc == WIFI_AUTH_WPA_PSK) { secColor = TFT_CYAN; secText = "WPA"; }
            else if (enc == WIFI_AUTH_WPA2_PSK) { secColor = 0x3186; secText = "WPA2"; }
            
            // Badge arrondi
            M5.Lcd.fillRoundRect(272, y+5, 36, 15, 7, secColor);
            if (enc == WIFI_AUTH_OPEN) M5.Lcd.setTextColor(TFT_BLACK); else M5.Lcd.setTextColor(TFT_WHITE);
            M5.Lcd.setTextSize(1);
            M5.Lcd.drawString(secText, 290, y + (rowH/2), 1); // Font 1 petite pour badge

            y += rowH;
        }
        M5.Lcd.setTextDatum(TL_DATUM);
    }
}

// ==========================================
// === LOGIQUE D'INTERFACE ===
// ==========================================
void DisplayManager::refreshUI() {
    if (currentTab == 0) showWiFi();
    else if (currentTab == 1) showConfig();
    else if (currentTab == 2) showStatus();
    else if (currentTab == 3) showScan();
    else if (currentTab == 4) showSTA();
    else if (currentTab == 5) showReset();
}

void DisplayManager::setCurrentTab(int tab) {
    if (tab >= 0 && tab < 6) {
        currentTab = tab;
    }
}

int DisplayManager::getCurrentTab() {
    return currentTab;
}

int DisplayManager::update() {
    // --- Gestion Mode Message (Prioritaire) ---
    if (isMessageMode) {
        if (M5.Touch.ispressed()) {
            Point p = M5.Touch.getPressPoint();
            lastActivity = millis(); // Reset screensaver

            // Bouton FERMER (Zone bas > 200)
            if (p.y > 200) {
                // Feedback bouton "Pro" (Inversion couleurs thème)
                M5.Lcd.fillRect(0, 200, 320, 40, THEME_ACCENT); 
                M5.Lcd.setTextColor(TFT_BLACK);
                M5.Lcd.setTextDatum(MC_DATUM);
                M5.Lcd.drawString("FERMER", 160, 220, 2); // Reste "FERMER"
                delay(100);

                // Animation de fermeture (Effacement vers le bas)
                for (int i = 0; i < 240; i += 20) {
                    M5.Lcd.fillRect(0, i, 320, 20, THEME_BG);
                    delay(5);
                }

                isMessageMode = false;
                drawHeader(); // Restaure le bandeau du haut (Batterie/Heure)
                drawTabs();   // Restaure les onglets du bas
                refreshUI(); // Retour à l'écran précédent
                return -1;
            }

            // Scroll Tactile
            if (lastTouchY != -1) {
                int dy = lastTouchY - p.y;
                messageScroll += dy;

                // Limite supérieure (ne pas dépasser le début)
                if (messageScroll < 0) messageScroll = 0;

                // Limite inférieure (ne pas dépasser la fin du texte)
                int viewHeight = 160; // Hauteur de la zone de texte visible
                int maxScroll = totalMessageHeight - viewHeight;
                if (maxScroll < 0) maxScroll = 0; // Si le texte est plus court que la vue
                if (messageScroll > maxScroll) messageScroll = maxScroll;

                drawMessageScreen();
            }
            lastTouchY = p.y;
        } else {
            lastTouchY = -1; // Reset du drag quand on lache
        }
        return -1; // On bloque le reste de l'interface tant que le message est ouvert
    }

    // --- Gestion Screensaver (Auto-Dim) ---
    if (M5.Touch.ispressed()) {
        lastActivity = millis(); // Reset du timer d'inactivité
        
        // Réveil si l'écran était sombre
        if (isDimmed) {
            M5.Lcd.setBrightness(128); // Retour luminosité normale
            isDimmed = false;
        }
        
        Point p = M5.Touch.getPressPoint();
        
        // Gestion des onglets (Zone Y > 215)
        if (p.y > 215) {
            int newTab = p.x / (320 / 6); // Ajustement de la largeur tactile
            if (newTab >= 0 && newTab < 6 && newTab != currentTab) {
                setCurrentTab(newTab);
                drawTabs();
                fadeTransition();  // Animation de transition
                refreshUI();
                return currentTab;
            }
        }

        // --- Gestion BOUTON CONFIGURATION (Onglet 1) ---
        if (currentTab == 1 && p.y >= 60 && p.y <= 95) {
             // Cycle des modes : AP -> STA -> AP_STA -> AP
             String m = config.wifi_mode;
             if (m == "AP") config.wifi_mode = "STA";
             else if (m == "STA") config.wifi_mode = "AP_STA";
             else config.wifi_mode = "AP";
             
             config.save();
             
             // Feedback visuel
             M5.Lcd.fillRoundRect(10, 60, 300, 35, 6, THEME_ACCENT);
             M5.Lcd.setTextColor(TFT_BLACK);
             M5.Lcd.setTextDatum(MC_DATUM);
             M5.Lcd.drawString("RESEAU: " + config.wifi_mode, 160, 78, 2);
             delay(200);
             
             // On signale au main.cpp qu'il faut recharger le WiFi via le drapeau needReloadWifi
             // (Note: needReloadWifi est dans main.cpp, ici on force juste un refresh UI en attendant)
             refreshUI(); 
             // Pour que le changement soit effectif, il faudra appuyer sur BtnA (Toggle) ou redémarrer, 
             // ou on peut ajouter une méthode callback. Ici l'utilisateur appuiera sur BtnA pour appliquer.
        }
        
        // Gestion du bouton "Rescan" (Onglet 3 uniquement, Zone Y 190-215)
        if (currentTab == 3) {
            // Zone Bouton Actualiser
            if (p.y >= 190 && p.y <= 215) {
                // Animation bouton pressé (Pill)
                M5.Lcd.fillRoundRect(60, 192, 200, 22, 11, THEME_ACCENT);
                M5.Lcd.setTextColor(TFT_BLACK);
                M5.Lcd.setTextDatum(MC_DATUM);
                M5.Lcd.drawString("ACTUALISER", 160, 204, 2);
                
                // CORRECTION BUG : Attendre que l'utilisateur relâche le doigt
                while(M5.Touch.ispressed()) { M5.update(); delay(10); }
                
                refreshUI(); // Relance le scan
                return -1;
            }

            // Zone Liste (Scroll Tactile)
            if (p.y > 60 && p.y < 190 && lastTouchY != -1) {
                int dy = lastTouchY - p.y;
                // Seuil de sensibilité
                if (abs(dy) > 5) {
                    int n = WiFi.scanComplete(); // Récupère le nombre du dernier scan
                    if (n > 5) {
                        if (dy > 0) scanScroll++;
                        else scanScroll--;
                        
                        // Bornage
                        if (scanScroll < 0) scanScroll = 0;
                        if (scanScroll > n - 5) scanScroll = n - 5;
                        
                        drawScanList(n);
                        lastTouchY = p.y; // Reset position pour fluidité
                        return -1;
                    }
                }
            }
        }
    } else {
        // Pas de touche pressée : vérification du délai
        // 30000 ms = 30 secondes avant de baisser la luminosité
        if (!isDimmed && (millis() - lastActivity > 30000)) {
            M5.Lcd.setBrightness(40); // Mode économie (sombre mais lisible)
            isDimmed = true;
        }
    }
    return -1;
}

// ==========================================
// === AFFICHAGE MESSAGES / POPUPS ===
// ==========================================
void DisplayManager::showReceivedMessage(const String& msg) {
    // Active le mode message plein écran avec défilement
    isMessageMode = true;
    messageText = msg;
    messageScroll = 0;
    lastTouchY = -1;

    // --- Calcul de la hauteur totale du texte ---
    // On utilise une zone "virtuelle" hors écran pour que M5.Lcd.print()
    // calcule le word-wrapping et nous donne la hauteur finale.
    M5.Lcd.setTextSize(2); // Utiliser une police lisible
    M5.Lcd.setCursor(10, 250); // Position de départ virtuelle (hors écran)
    M5.Lcd.print(messageText);
    int finalY = M5.Lcd.getCursorY();
    totalMessageHeight = finalY - 250;

    drawMessageScreen();
}

void DisplayManager::setAckStatus(bool success) {
    lastAckSuccess = success;
    lastAckTime = millis();
}

void DisplayManager::drawMessageScreen() {
    // Header Message
    M5.Lcd.fillRect(0, 0, 320, 30, THEME_ACCENT);
    M5.Lcd.setTextColor(TFT_BLACK);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("NOUVEAU MESSAGE", 160, 15, 2);
    
    // Zone Texte (Nettoyage de la zone centrale)
    M5.Lcd.fillRect(0, 30, 320, 170, THEME_BG);
    
    // Texte défilant
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextSize(2); // Font 2 (16px) est un bon compromis lisibilité/densité
    M5.Lcd.setCursor(10, 40 - messageScroll);
    M5.Lcd.print(messageText);

    // Footer Bouton (Fixe - redessiné par dessus le texte pour le cacher)
    M5.Lcd.fillRect(0, 200, 320, 40, 0x18E3);
    M5.Lcd.drawFastHLine(0, 200, 320, THEME_ACCENT);
    M5.Lcd.setTextColor(THEME_ACCENT);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("FERMER", 160, 220, 2);
    M5.Lcd.setTextDatum(TL_DATUM);
}

void DisplayManager::showConnecting(const String& message) {
    clearContent();
    M5.Lcd.fillRoundRect(40, 70, 240, 100, 8, THEME_BOX);
    M5.Lcd.drawRoundRect(40, 70, 240, 100, 8, THEME_ACCENT);

    M5.Lcd.setTextColor(TFT_YELLOW);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("CONNEXION EN COURS", 160, 95, 2);
    
    M5.Lcd.setTextColor(TFT_WHITE);
    int newlineIndex = message.indexOf('\n');
    M5.Lcd.drawString(message.substring(0, newlineIndex), 160, 120, 2);
    M5.Lcd.drawString(message.substring(newlineIndex + 1), 160, 140, 2);
    M5.Lcd.setTextDatum(TL_DATUM);
}
