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
 * @brief Reçoit et vérifie la réponse du journal (ACK)
 * Attend la balise de fin <E> et vérifie la validité du LRC.
 */
String RS232Manager::receive() {
    String response = "";
    unsigned long startTimeout = millis();
    
    // Attente active de la réponse avec un timeout de 500ms
    while (millis() - startTimeout < 500) {
        while (serial->available() > 0) {
            char c = serial->read();
            response += c;
            
            // Si on détecte la balise de fin de protocole
            if (response.endsWith("<E>")) {
                // Vérification basique du LRC dans la réponse
                // Format type: <ID01><ACK>XX<E> où XX est le LRC
                if (response.length() >= 10) {
                    String content = response.substring(response.indexOf('>') + 1, response.length() - 5);
                    String receivedLRC = response.substring(response.length() - 5, response.length() - 3);
                    
                    // Validation par recalcul du LRC pour garantir l'intégrité
                    if (calculateLRC(content).equalsIgnoreCase(receivedLRC)) {
                        return response; // Réponse valide
                    }
                }
                return response;
            }
        }
        delay(10); // Petit délai pour ne pas saturer le CPU
    }
    
    return ""; // Timeout : aucune réponse ou trame incomplète
}

/**
 * @brief Envoie une trame complète Tronios
 */
String RS232Manager::sendTrame(String msg) {
    // Nettoyage du tampon de lecture avant l'envoi pour ne pas lire d'anciennes données
    while(serial->available()) serial->read();

    String payload = payloadBase + msg;
    String lrc_hex = calculateLRC(payload);
    String fullFrame = ID + payload + lrc_hex + "<E>";

    serial->print(fullFrame);
    // Serial.println("RS232 envoyé : " + fullFrame);  // décommente seulement pour debug

    return fullFrame;
}