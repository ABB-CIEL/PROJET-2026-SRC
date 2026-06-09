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

// ============================================================
// REGION : Déchiffrement (Intégration E1)
// ============================================================

String CRS232Manager::hexToString(String hex) {
    String result = "";
    hex.trim();
    hex.replace(" ", "");
    hex.replace("\n", "");
    hex.replace("\r", "");
    for (int i = 0; i < (int)hex.length(); i += 2) {
        if (i + 2 <= (int)hex.length()) {
            String hexByte = hex.substring(i, i + 2);
            char byteChar = (char)strtol(hexByte.c_str(), NULL, 16);
            result += byteChar;
        }
    }
    return result;
}

String CRS232Manager::dechiffrerCesar(String data, int decalage) {
    String result = "";
    for (int i = 0; i < (int)data.length(); i++) {
        unsigned char c = data[i];
        c = (c - decalage + 256) % 256;
        result += (char)c;
    }
    return result;
}

String CRS232Manager::dechiffrerXOR(String data, uint8_t key) {
    String result = "";
    for (int i = 0; i < (int)data.length(); i++) {
        unsigned char c = data[i];
        c = c ^ key;
        result += (char)c;
    }
    return result;
}

long long CRS232Manager::powMod(long long base, long long exp, long long mod) {
    long long res = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (res * base) % mod;
        base = (base * base) % mod;
        exp = exp / 2;
    }
    return res;
}

String CRS232Manager::processE1Frame(String trame) {
    int debut = trame.indexOf('[');
    int fin = trame.indexOf(']');
    
    if (debut >= 0 && fin > debut) {
        String enTete = trame.substring(debut, fin + 1);
        String donnees = trame.substring(fin + 1);
        donnees.trim();

        if (enTete.startsWith("[CESAR:")) {
            int split = enTete.indexOf(':');
            int decal = enTete.substring(split + 1, fin).toInt();
            // César est encodé une seule fois en hex par l'IHM
            return dechiffrerCesar(hexToString(donnees), decal);
        } 
        else if (enTete.startsWith("[XOR:")) {
            int split = enTete.indexOf(':');
            String hexKey = enTete.substring(split + 1, fin);
            uint8_t key = (uint8_t)strtol(hexKey.c_str(), NULL, 16);
            
            // On convertit l'hexadécimal en octets, puis on applique le XOR
            return dechiffrerXOR(hexToString(donnees), key);
        }
        else if (enTete.startsWith("[RSA:")) {
            // RSA utilise des blocs hex de 4 caractères dans l'IHM Qt
            long long n = 8357;
            long long d = 4663; // Clé privée correspondant à e=7
            String result = "";
            donnees.replace(" ", "");
            
            for (int i = 0; i + 4 <= (int)donnees.length(); i += 4) {
                String part = donnees.substring(i, i + 4);
                unsigned short c = (unsigned short)strtol(part.c_str(), NULL, 16);
                result += (char)powMod(c, d, n);
            }
            return result;
        }
        else if (enTete.startsWith("[CLAIR]")) {
            return donnees;
        }
    }
    return trame; // Retourne brut si pas d'en-tête
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