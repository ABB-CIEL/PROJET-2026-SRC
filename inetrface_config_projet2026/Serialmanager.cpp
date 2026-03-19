#include "serialmanager.h"

SerialManager::SerialManager(QObject *parent) : QObject(parent) {
    connect(&serial, &QSerialPort::readyRead, this, [this]() {
        QString data = serial.readAll();
        emit dataReceived(data);
    });
}

bool SerialManager::openPort(const QString &portName, int baudRate) {
    serial.setPortName(portName);
    serial.setBaudRate(baudRate);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    return serial.open(QIODevice::ReadWrite);
}

void SerialManager::closePort() {
    if (serial.isOpen()) serial.close();
}

void SerialManager::sendMessage(const QString &message) {
    if (serial.isOpen()) serial.write(message.toUtf8());
}
