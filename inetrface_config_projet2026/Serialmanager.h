#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>

class SerialManager : public QObject {
    Q_OBJECT

public:
    explicit SerialManager(QObject *parent = nullptr);
    bool openPort(const QString &portName, int baudRate = 9600);
    void closePort();
    void sendMessage(const QString &message);

signals:
    void dataReceived(const QString &data);

private:
    QSerialPort serial;
};

#endif
