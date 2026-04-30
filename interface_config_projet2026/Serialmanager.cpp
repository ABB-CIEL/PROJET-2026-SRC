/**
 * @file    serialmanager.cpp
 * @brief   Implémentation de la classe SerialManager.
 * @date    2026
 */

#include "serialmanager.h"

/**
 * @brief Constructeur : connecte le signal readyRead à l'émission de dataReceived.
 */
SerialManager::SerialManager(QObject *parent) : QObject(parent) {
    connect(&serial, &QSerialPort::readyRead, this, [this]() {
        QString data = serial.readAll();
        emit dataReceived(data);
    });
}

/**
 * @brief  Ouvre le port série en 8N1, sans contrôle de flux.
 * @param  portName Nom du port.
 * @param  baudRate Vitesse en bauds.
 * @return true si l'ouverture a réussi, false sinon.
 */
bool SerialManager::openPort(const QString &portName, int baudRate) {
    serial.setPortName(portName);
    serial.setBaudRate(baudRate);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    return serial.open(QIODevice::ReadWrite);
}

/**
 * @brief Ferme le port série s'il est ouvert.
 */
void SerialManager::closePort() {
    if (serial.isOpen()) serial.close();
}

/**
 * @brief Envoie un message UTF-8 sur le port (uniquement si le port est ouvert).
 * @param message Chaîne à émettre.
 */
void SerialManager::sendMessage(const QString &message) {
    if (serial.isOpen()) serial.write(message.toUtf8());
}
