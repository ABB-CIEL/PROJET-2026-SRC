/**
 * @file main.cpp
 * @brief Orchestration principale du projet Passerelle WiFi/RS232
 * @author Étudiant X
 * @date 2026
 */

// ============================================================
// REGION : Inclusions et Déclarations
// ============================================================

#include <Arduino.h>
#include <M5Core2.h>
#include <ArduinoJson.h>
#include <time.h>

#include "RS232Manager.h"
#include "DebugManager.h"
#include "ConfigManager.h"
#include "WifiManager.h"
#include "UDPManager.h"
#include "DisplayManager.h"

HardwareSerial SerialRS232(2);

CRS232Manager rs232(SerialRS232);
CDebugManager debug;
CConfigManager config;
CWifiManager wifiManager;
CUDPManager udpManager;
CDisplayManager displayManager(config, wifiManager);

bool needReloadWifi = false; // Drapeau pour redémarrage différé du WiFi

// ============================================================
// REGION : Gestion de la connectivité WiFi
// ============================================================

/**
 * @brief Configure le gestionnaire WiFi à partir des données de configuration
 */
void applyWifiConfig()
{
    wifiManager.setApCredentials(config.ap_ssid, config.ap_password);
    wifiManager.setStaCredentials(config.sta_ssid, config.sta_password);
    wifiManager.setMode(config.wifi_mode);
    
    IPAddress ip, gw, mask, dns;
    ip.fromString(config.ip);
    gw.fromString(config.gateway);
    mask.fromString(config.subnet);
    dns.fromString(config.dns);
    wifiManager.setNetwork(ip, gw, mask, dns);
}

/**
 * @brief Passe le module Wi‑Fi de l'état actif à inactif et inversement.
 *
 * Si on coupe la connexion on arrête également le service UDP, sinon on
 * réinitialise la configuration et relance le serveur UDP en mode STA.
 * Met à jour l'interface graphique à chaque changement d'état.
 */
void toggleWifi()
{
    if (wifiManager.isActive()) {
        // coupure
        wifiManager.deactivate();
        udpManager.stop();
        delay(500);
    } else {
        // activation avec config
        applyWifiConfig();
        // Affiche un écran d'attente SEULEMENT si on passe en mode STA
        if (config.wifi_mode == "STA") {
            displayManager.showConnecting("Connexion à :\n" + config.sta_ssid);
        }
        delay(200);
        if (wifiManager.activate()) {
            // On démarre le serveur UDP dans les deux modes (AP et STA) si l'activation a réussi
            udpManager.begin(config.udp_port, config.config_port);
        }
    }
    displayManager.refreshUI();
}

// Fonction dédiée pour recharger le WiFi sans bloquer la réponse JSON
void reloadWifiConfig()
{
    wifiManager.deactivate();
    delay(500); // Pause pour bien libérer le hardware
    applyWifiConfig();
    // Affiche un écran d'attente SEULEMENT si on passe en mode STA
    if (config.wifi_mode == "STA") {
        displayManager.showConnecting("Connexion à :\n" + config.sta_ssid);
    }
    if (wifiManager.activate()) {
        udpManager.begin(config.udp_port, config.config_port);
    }
    displayManager.refreshUI();
}

/**
 * @brief Traite une chaîne JSON provenant de l'interface série ou réseau.
 *
 * Le format attendu est {"cmd":"<commande>",...}. Les commandes
 * prises en charge :
 *   - "get_config"  : renvoie la configuration courante
 *   - "set_config"  : met à jour la configuration et la sauvegarde
 *   - "reboot"      : redémarre le module
 *
 * En cas d'erreur de désérialisation ou de commande inconnue, un JSON
 * d'erreur est renvoyé.
 *
 */
String processJsonCommand(String jsonInput)
{
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, jsonInput);
    DynamicJsonDocument res(2048);

    if (error)
    {
        res["status"] = "ERROR";
        res["message"] = "JSON invalide";
    }
    else
    {
        String cmd(doc["cmd"] | "");
        
        for (int i = 0; i < cmd.length(); i++)
        {
            if (cmd[i] >= 'A' && cmd[i] <= 'Z')
            {
                cmd[i] = cmd[i] - 'A' + 'a';
            }
        }

        if (cmd == "get_config")
        {
            res["status"] = "OK";
            res["mode"] = config.wifi_mode;
            res["ap_ssid"] = config.ap_ssid;
            res["ap_password"] = config.ap_password;
            res["sta_ssid"] = config.sta_ssid;
            res["sta_password"] = config.sta_password;
            
            res["udp_port"] = config.udp_port;
            res["config_port"] = config.config_port;
            res["baud_rate"] = config.baud_rate;
            res["data_bits"] = config.data_bits;
            res["parity"] = config.parity;
            res["stop_bits"] = config.stop_bits;
            res["serial_type"] = config.serial_type;

            res["ip"] = config.ip;
            res["gateway"] = config.gateway;
            res["mask"] = config.subnet;
            res["dns"] = config.dns;
            res["mac"] = WiFi.macAddress();
        }
        else if (cmd == "reboot")
        {
            res["status"] = "OK";
            String r; serializeJson(res, r);
            Serial.println(r);
            delay(200);
            ESP.restart();
        }
        else if (cmd == "getmac")
        {
            res["status"] = "OK";
            res["mac"] = WiFi.macAddress();
        }
        else if (cmd == "set_config")
        {
            if (doc["mode"].is<String>())
            {
                String m = doc["mode"].as<String>();
                m.trim();
                m.toUpperCase();
                if (m == "STA" || m == "AP" || m == "AP_STA")
                {
                    config.wifi_mode = m;
                    if (m == "STA")
                    {
                        displayManager.setCurrentTab(4);
                    }
                    else
                    {
                        displayManager.setCurrentTab(0);
                    }
                }
            }
            
            if (doc["ap_ssid"].is<String>())
            {
                config.ap_ssid = doc["ap_ssid"].as<String>();
            }
            
            if (doc["ap_password"].is<String>())
            {
                config.ap_password = doc["ap_password"].as<String>();
            }
            else if (doc["ap_pass"].is<String>())
            {
                config.ap_password = doc["ap_pass"].as<String>();
            }

            if (doc["sta_ssid"].is<String>())
            {
                config.sta_ssid = doc["sta_ssid"].as<String>();
            }
            
            if (doc["sta_password"].is<String>())
            {
                config.sta_password = doc["sta_password"].as<String>();
            }
            else if (doc["sta_pass"].is<String>())
            {
                config.sta_password = doc["sta_pass"].as<String>();
            }

            if (doc["udp_port"].is<int>()) config.udp_port = doc["udp_port"];
            if (doc["config_port"].is<int>()) config.config_port = doc["config_port"];

            if (doc["baud_rate"].is<int>())  config.baud_rate = doc["baud_rate"];
            if (doc["data_bits"].is<int>())  config.data_bits = doc["data_bits"];
            if (doc["parity"].is<String>())  config.parity = doc["parity"].as<String>();
            if (doc["stop_bits"].is<int>())  config.stop_bits = doc["stop_bits"];
            if (doc["serial_type"].is<String>()) config.serial_type = doc["serial_type"].as<String>();
            
            if (doc["ip"].is<String>())      config.ip = doc["ip"].as<String>();
            if (doc["mask"].is<String>())    config.subnet = doc["mask"].as<String>();
            if (doc["gw"].is<String>())      config.gateway = doc["gw"].as<String>();
            if (doc["dns"].is<String>())     config.dns = doc["dns"].as<String>();

            config.save();
            needReloadWifi = true;
            res["status"] = "OK";
        }
        else
        {
            res["status"] = "ERROR";
            res["message"] = "Commande inconnue";
        }
    }

    String response;
    serializeJson(res, response);
    return response;
}

void setup()
{
    M5.begin(true, true, true, true);
    M5.Axp.SetLed(false);           

    displayManager.showSplashScreen();
    debug.begin(115200);

    Serial.end();                   
    delay(2000);                    

    Serial.begin(115200);           
    Serial.setRxBufferSize(4096);

    for (int i = 0; i < 50; i++)
    {
        while (Serial.available() > 0)
        {
            Serial.read();
        }
        delay(30);
    }

    Serial.println("\n\n=== M5 READY ===");
    delay(100);
    Serial.println("M5 READY");
    delay(100);
    Serial.println("M5 READY");

    config.load();
    
    rs232.begin(config.baud_rate); 
    
    uint32_t serialConfig = rs232.convertConfig(config.data_bits, config.parity, config.stop_bits);
    SerialRS232.begin(config.baud_rate, serialConfig, 13, 14);

    applyWifiConfig();
    if (wifiManager.activate())
    {
        udpManager.begin(config.udp_port, config.config_port);
    }

    displayManager.drawHeader();
    displayManager.drawTabs();
    displayManager.refreshUI();
}

void loop()
{
    M5.update();

    if (needReloadWifi)
    {
        needReloadWifi = false;
        reloadWifiConfig();
    }

    int newTab = displayManager.update();
    if (newTab != -1)
    {
        delay(300);
    }

    if (M5.BtnA.wasPressed())
    {
        toggleWifi();
    }
    if (M5.BtnC.isPressed() && M5.BtnC.pressedFor(3000))
    {
        config.factoryReset();
    }

    if (Serial.available() > 0)
    {
        String serialInput = Serial.readStringUntil('\n');
        serialInput.trim();
        if (serialInput.length() > 0 && serialInput[0] == '{')
        {
            String response = processJsonCommand(serialInput);
            Serial.println(response);
        }
    }

    if (wifiManager.isActive())
    {
        String msg;
        if (udpManager.receiveJournal(msg))
        {
            String frameSent = rs232.send(msg);
            debug.log("Envoi RS232: " + frameSent);

            String ack = rs232.receive();
            if (ack.length() > 0)
            {
                debug.log("ACK Journal reçu: " + ack);
            }
            else
            {
                debug.log("ERREUR: Pas d'acquittement du journal !");
            }

            if (displayManager.getCurrentTab() == 0 || displayManager.getCurrentTab() == 4)
            {
                displayManager.showReceivedMessage(msg);
            }
        }
        IPAddress remoteIP;
        int remotePort;
        if (udpManager.receiveConfig(msg, remoteIP, remotePort))
        {
            String response = processJsonCommand(msg);
            udpManager.sendConfigResponse(remoteIP, remotePort, response);
        }
    }

    static uint32_t last = 0;
    if (millis() - last > 1000)
    {
        displayManager.drawHeader();
        last = millis();
    }

    delay(10);
}