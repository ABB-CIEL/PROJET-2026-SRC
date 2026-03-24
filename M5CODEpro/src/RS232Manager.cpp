#include "RS232Manager.h"

RS232Manager::RS232Manager(HardwareSerial& port) {
    serial = &port;
    ID = "<ID01>";
    payloadBase = "<L1><PA><FA><Ma><WC><FA><CA>";
}

/**
 * @brief Calcule le checksum LRC
 */
String RS232Manager::calculateLRC(String data) {
  byte lrc = 0;
  for (unsigned int i = 0; i < data.length(); i++) {
    lrc ^= data[i];
  }
  char hex[3];
  sprintf(hex, "%02X", lrc);
  return String(hex);
}

/**
 * @brief Convertit les paramètres de config en constante Arduino SerialConfig
 * Déplacé depuis main.cpp pour une meilleure organisation.
 */
uint32_t RS232Manager::convertConfig(int db, String p, int sb) {
    // Mapping basique pour les cas courants (8N1, 8E1, etc.)
    if (db == 8 && p == "N" && sb == 1) return SERIAL_8N1;
    if (db == 8 && p == "N" && sb == 2) return SERIAL_8N2;
    if (db == 8 && p == "E" && sb == 1) return SERIAL_8E1;
    if (db == 8 && p == "E" && sb == 2) return SERIAL_8E2;
    if (db == 8 && p == "O" && sb == 1) return SERIAL_8O1;
    if (db == 8 && p == "O" && sb == 2) return SERIAL_8O2;
    
    if (db == 7 && p == "N" && sb == 1) return SERIAL_7N1;
    if (db == 7 && p == "N" && sb == 2) return SERIAL_7N2;
    if (db == 7 && p == "E" && sb == 1) return SERIAL_7E1;
    if (db == 7 && p == "E" && sb == 2) return SERIAL_7E2;
    if (db == 7 && p == "O" && sb == 1) return SERIAL_7O1;
    if (db == 7 && p == "O" && sb == 2) return SERIAL_7O2;

    return SERIAL_8N1; // Défaut
}

void RS232Manager::begin(int baud) {
    baudRate = baud;
    // On ne fait rien ici car main.cpp initialise déjà le port avec la config complète (parité, stop bits...)
    // serial->begin(baudRate, SERIAL_8N1, 13, 14); 
}

/**
 * @brief Envoie n'importe quel message (format complet avec LRC)
 */
String RS232Manager::send(String msg) {
    if (msg.length() == 0) return "";
    return sendTrame(msg);
}

/**
 * @brief Reçoit les données (si disponibles)
 */
String RS232Manager::receive() {
    if (serial->available()) {
        return serial->readStringUntil('\n');
    }
    return "";
}

/**
 * @brief Envoie une trame complète Tronios
 */
String RS232Manager::sendTrame(String msg) {
    String payload = payloadBase + msg;
    String lrc_hex = calculateLRC(payload);
    String fullFrame = ID + payload + lrc_hex + "<E>";

    serial->print(fullFrame);
    // Serial.println("RS232 envoyé : " + fullFrame);  // décommente seulement pour debug

    return fullFrame;
}

/* =============================================
   === NOUVEAU : SYSTEME D'ANIMATIONS PRÊT POUR MENU ===
   ============================================= */

void RS232Manager::sendAnimation(int animIndex, String customText) {
    String command;

    switch (animIndex) {
        case 0:  command = "1/B = X";                    break; // Scroll Gauche Rapide - Rouge
        case 1:  command = "4/E = X";                    break; // Scroll Droite - Vert
        case 2:  command = "2/C = X";                    break; // Scroll Haut - Orange
        case 3:  command = "9/J = X";                    break; // Scroll Bas - Violet
        case 4:  command = "10/K = X";                   break; // Effet NEIGE (le plus beau !)
        case 5:  command = "16/Q = X";                   break; // Vitesse 3 - Jaune
        case 6:  command = "q = X";                      break; // Vitesse 1 (très lent) - Bleu
        case 7:  command = "8/I = X";                    break; // Scroll Up
        case 8:  command = "3/D = X";                    break; // Scroll Left page 3
        case 9:  command = "5/F = X";                    break; // Scroll Right Orange
        case 10: command = "0/A = X";                    break; // Effet basique ID 0

        default: command = "1/B = X";                    break;
    }

    // Si l'utilisateur veut un texte personnalisé
    if (customText.length() > 0 && animIndex != 4 && animIndex != 10) {
        command = command.substring(0, 5) + customText;  // remplace le "X" par le texte
    }

    send(command);
}

// Version simplifiée pour le menu (texte libre)
void RS232Manager::sendCustomText(String text, int animIndex) {
    String base = "1/B = ";           // par défaut scroll gauche
    if (animIndex == 4) base = "10/K = ";   // effet neige
    if (animIndex == 1) base = "4/E = ";
    send(base + text);
}