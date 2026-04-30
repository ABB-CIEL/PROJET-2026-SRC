/**
 * @file UDPManager.cpp
 * @brief Implémentation de la gestion des flux UDP (Journal et Configuration)
 * @author Étudiant BTS CIEL IR
 * @date 2026
 */

#include "UDPManager.h"

// ============================================================
// REGION : Constructeur et Initialisation
// ============================================================

/** @class CUDPManager
 *  @brief Gère les communications UDP sur deux ports distincts
 */
CUDPManager::CUDPManager()
{
    this->journalPort = 1470;
    this->configPort = 1000;
}

/**
 * @brief Initialise les serveurs UDP
 * @param portJournal Port pour les messages du journal
 * @param portConfig Port pour les commandes JSON
 */
void CUDPManager::begin(int portJournal, int portConfig)
{
    this->journalPort = portJournal;
    this->configPort = portConfig;

    this->udpJournal.begin(this->journalPort);
    this->udpConfig.begin(this->configPort);
}

// ============================================================
// REGION : Réception et Envoi
// ============================================================

/**
 * @brief Reçoit un message destiné au journal
 * @param msg Référence de la chaîne pour stocker le message
 * @return true si un message a été reçu
 */
bool CUDPManager::receiveJournal(String &msg)
{
    int packetSize = this->udpJournal.parsePacket();
    if (!packetSize)
    {
        return false;
    }

    char buffer[256];
    int len = this->udpJournal.read(buffer, sizeof(buffer) - 1);

    if (len <= 0)
    {
        return false;
    }

    buffer[len] = 0;
    msg = String(buffer);
    return true;
}

/**
 * @brief Reçoit une commande de configuration
 * @param msg Contenu JSON
 * @param ip IP de l'expéditeur (pour réponse)
 * @param port Port de l'expéditeur
 * @return true si reçu
 */
bool CUDPManager::receiveConfig(String &msg, IPAddress &ip, int &port)
{
    int packetSize = this->udpConfig.parsePacket();
    if (!packetSize)
    {
        return false;
    }

    char buffer[512];
    int len = this->udpConfig.read(buffer, sizeof(buffer) - 1);

    if (len <= 0)
    {
        return false;
    }

    buffer[len] = 0;
    msg = String(buffer);
    ip = this->udpConfig.remoteIP();
    port = this->udpConfig.remotePort();
    return true;
}

/**
 * @brief Répond à une commande de configuration
 */
void CUDPManager::sendConfigResponse(IPAddress ip, int port, String message)
{
    this->udpConfig.beginPacket(ip, port);
    this->udpConfig.print(message);
    this->udpConfig.endPacket();
}

/**
 * @brief Arrête les services UDP
 */
void CUDPManager::stop()
{
    this->udpJournal.stop();
    this->udpConfig.stop();
}
