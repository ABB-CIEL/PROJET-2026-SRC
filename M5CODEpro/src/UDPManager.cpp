#include "UDPManager.h"

UDPManager::UDPManager() {

    journalPort = 1470;
    configPort = 1000;

}

void UDPManager::begin(int portJournal, int portConfig) {

    journalPort = portJournal;
    configPort = portConfig;

    udpJournal.begin(journalPort);
    udpConfig.begin(configPort);

}

bool UDPManager::receiveJournal(String &msg) {

    int packetSize = udpJournal.parsePacket();

    if (!packetSize)
        return false;

    char buffer[256];

    int len = udpJournal.read(buffer, sizeof(buffer) - 1);

    if (len <= 0)
        return false;

    buffer[len] = 0;

    msg = String(buffer);

    return true;
}

bool UDPManager::receiveConfig(String &msg, IPAddress &ip, int &port) {

    int packetSize = udpConfig.parsePacket();

    if (!packetSize)
        return false;

    char buffer[512];

    int len = udpConfig.read(buffer, sizeof(buffer) - 1);

    if (len <= 0)
        return false;

    buffer[len] = 0;

    msg = String(buffer);

    ip = udpConfig.remoteIP();
    port = udpConfig.remotePort();

    return true;
}

void UDPManager::sendConfigResponse(IPAddress ip, int port, String message) {

    udpConfig.beginPacket(ip, port);
    udpConfig.print(message);
    udpConfig.endPacket();

}

void UDPManager::stop() {

    udpJournal.stop();
    udpConfig.stop();

}
