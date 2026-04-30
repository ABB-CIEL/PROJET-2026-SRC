/**
 * @file RS232Manager.cpp
 * @brief Implémentation de la gestion du protocole RS232 pour le journal Tronios
 * @author Étudiant BTS CIEL IR
 * @date 2026
 */

#include "RS232Manager.h"

// ============================================================
// REGION : Constructeur et Initialisation
// ============================================================

/** @class CRS232Manager
 *  @brief Gère la communication série avec le protocole spécifique Tronios
 */
CRS232Manager::CRS232Manager(HardwareSerial& port)
{
    this->serial = &port;
    this->id = "<ID01>";
    this->payloadBase = "<L1><PA><FA><Ma><WC><FA><CA>";
}

/**
 * @brief Initialise les paramètres de base de la communication
 * @param baud Vitesse de communication en bauds
 */
void CRS232Manager::begin(int baud)
{
    this->baudRate = baud;
}

// ============================================================
// REGION : Outils de Calcul et Configuration
// ============================================================

/**
 * @brief Calcule le checksum LRC (Longitudinal Redundancy Check)
 * @param data Chaîne de caractères dont on veut calculer le LRC
 * @return Valeur du LRC formatée en chaîne hexadécimale (2 caractères)
 */
String CRS232Manager::calculateLRC(String data)
{
    byte lrc = 0;
    for (unsigned int i = 0; i < data.length(); i++)
    {
        lrc ^= data[i];
    }
    char hex[3];
    sprintf(hex, "%02X", lrc);
    return String(hex);
}

/**
 * @brief Convertit les paramètres entiers/chaînes en constantes SerialConfig
 * @param db Nombre de bits de données (7 ou 8)
 * @param p Parité ("N", "E" ou "O")
 * @param sb Nombre de bits de stop (1 ou 2)
 * @return Constante de type uint32_t compatible avec Serial.begin()
 */
uint32_t CRS232Manager::convertConfig(int db, String p, int sb)
{
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

    return SERIAL_8N1;
}

// ============================================================
// REGION : Envoi et Réception de Trames
// ============================================================

/**
 * @brief Prépare et envoie un message au journal lumineux
 * @param msg Texte brut à envoyer
 * @return La trame complète envoyée pour archivage/debug
 */
String CRS232Manager::send(String msg)
{
    if (msg.length() == 0)
    {
        return "";
    }
    return this->sendTrame(msg);
}

/**
 * @brief Attend et vérifie l'acquittement du journal
 * @return La réponse brute du journal ou une chaîne vide si erreur/timeout
 */
String CRS232Manager::receive()
{
    String response = "";
    unsigned long startTimeout = millis();
    const int TIMEOUT_MS = 500;

    while (millis() - startTimeout < TIMEOUT_MS)
    {
        while (this->serial->available() > 0)
        {
            char c = (char)this->serial->read();
            response += c;

            if (response.endsWith("<E>"))
            {
                if (response.length() >= 10)
                {
                    String content = response.substring(response.indexOf('>') + 1, response.length() - 5);
                    String receivedLRC = response.substring(response.length() - 5, response.length() - 3);

                    if (this->calculateLRC(content).equalsIgnoreCase(receivedLRC))
                    {
                        return response;
                    }
                }
                return response;
            }
        }
        delay(10);
    }
    return "";
}

/**
 * @brief Encapsule les données dans le format de trame Tronios
 * @param msg Texte à encapsuler
 * @return Trame complète formatée
 */
String CRS232Manager::sendTrame(String msg)
{
    while (this->serial->available())
    {
        this->serial->read();
    }

    String payload = this->payloadBase + msg;
    String lrcHex = this->calculateLRC(payload);
    String fullFrame = this->id + payload + lrcHex + "<E>";

    this->serial->print(fullFrame);

    return fullFrame;
}