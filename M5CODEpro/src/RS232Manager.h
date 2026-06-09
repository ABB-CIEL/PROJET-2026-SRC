/**
 * @file RS232Manager.h
 * @brief Définition de la classe de gestion du protocole RS232 Tronios
 * @author Étudiant BTS CIEL IR
 * @date 2026
 */

#ifndef RS232MANAGER_H
#define RS232MANAGER_H

#include <Arduino.h>
#include <HardwareSerial.h>

/** @class CRS232Manager
 *  @brief Gère la communication série avec le protocole spécifique Tronios
 */
class CRS232Manager
{
private:
    HardwareSerial* serial; ///< Port série matériel utilisé
    String id;              ///< Identifiant du journal (ex: <ID01>)
    String payloadBase;     ///< En-tête de protocole fixe
    int baudRate;           ///< Vitesse de transmission

    // Outils de déchiffrement (Intégration E1)
    String hexToString(String hex);
    String dechiffrerCesar(String data, int decalage);
    String dechiffrerXOR(String data, uint8_t key);
    long long powMod(long long base, long long exp, long long mod);

    /**
     * @brief Calcule le checksum LRC
     * @param data Chaîne de données
     * @return Checksum hexadécimal
     */
    String calculateLRC(String data);

    /**
     * @brief Formate et envoie la trame brute
     * @param msg Message à encapsuler
     * @return Trame finale
     */
    String sendTrame(String msg);

public:
    /**
     * @brief Constructeur
     * @param port Référence vers le port matériel (Serial2)
     */
    CRS232Manager(HardwareSerial& port);

    /**
     * @brief Initialisation
     * @param baud Débit en bauds
     */
    void begin(int baud);
    
    /**
     * @brief Convertit les paramètres en constante Arduino
     * @param dataBits 7 ou 8
     * @param parity "N", "E", "O"
     * @param stopBits 1 ou 2
     * @return Configuration Serial
     */
    uint32_t convertConfig(int dataBits, String parity, int stopBits);

    String send(String msg);
    String receive();

    /**
     * @brief Décode une trame provenant de l'IHM E1 (Qt)
     * @param trame Trame brute avec en-tête [ALGO:...]
     * @return Message déchiffré en clair
     */
    String processE1Frame(String trame);
};

#endif  