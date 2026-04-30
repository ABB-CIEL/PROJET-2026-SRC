/**
 * @file    serialmanager.h
 * @brief   Déclaration de la classe SerialManager (gestionnaire de port série).
 * @date    2026
 */

#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>

/**
 * @class   SerialManager
 * @brief   Encapsule un QSerialPort : ouverture, fermeture, envoi et réception asynchrone.
 */
class SerialManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief  Constructeur.
     * @param  parent Objet parent Qt (nullptr par défaut).
     */
    explicit SerialManager(QObject *parent = nullptr);

    /**
     * @brief  Ouvre un port série (8N1, sans contrôle de flux).
     * @param  portName Nom du port (ex : "COM3", "/dev/ttyUSB0").
     * @param  baudRate Vitesse de transmission en bauds (9600 par défaut).
     * @return true si l'ouverture a réussi, false sinon.
     */
    bool openPort(const QString &portName, int baudRate = 9600);

    /**
     * @brief  Ferme le port série s'il est ouvert.
     */
    void closePort();

    /**
     * @brief  Envoie un message texte sur le port série (encodé en UTF-8).
     * @param  message Chaîne à émettre.
     */
    void sendMessage(const QString &message);

signals:
    /**
     * @brief  Signal émis chaque fois que des données sont reçues sur le port.
     * @param  data Données reçues, converties en QString.
     */
    void dataReceived(const QString &data);

private:
    QSerialPort serial; /**< Port série encapsulé. */
};

#endif
