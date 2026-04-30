/**
 * @file UDPManager.h
 * @brief Définition de la classe de gestion des échanges UDP
 * @author Étudiant BTS CIEL IR
 * @date 2026
 */

#ifndef UDP_MANAGER_H
#define UDP_MANAGER_H

#include <Arduino.h>
#include <WiFiUdp.h>

/** @class CUDPManager
 *  @brief Gère les sockets UDP pour le journal et la configuration
 */
class CUDPManager
{
private:
    WiFiUDP udpJournal; ///< Socket pour les messages du journal
    WiFiUDP udpConfig;  ///< Socket pour les commandes JSON

    int journalPort;    ///< Port d'écoute journal
    int configPort;     ///< Port d'écoute config

public:
    CUDPManager();

    /**
     * @brief Démarre les services UDP
     * @param portJournal Port journal
     * @param portConfig Port config
     */
    void begin(int portJournal, int portConfig);

    /**
     * @brief Reçoit un message journal
     * @param msg Référence de sortie
     * @return true si reçu
     */
    bool receiveJournal(String &msg);

    /**
     * @brief Reçoit une commande config
     * @param msg JSON reçu
     * @param ip IP client
     * @param port Port client
     * @return true si reçu
     */
    bool receiveConfig(String &msg, IPAddress &ip, int &port);

    /**
     * @brief Envoie une réponse JSON
     */
    void sendConfigResponse(IPAddress ip, int port, String message);

    /**
     * @brief Arrête les sockets
     */
    void stop();
};

#endif 
