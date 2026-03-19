#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    portSerie = new QSerialPort(this);

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->ComboComPortSerial->addItem(info.portName());
        ui->ComboComPort->addItem(info.portName());
    }
    ui->progressBar_2->setMinimum(0);
    ui->progressBar_2->setMaximum(100);
    ui->progressBar_2->setValue(0);
}

MainWindow::~MainWindow()
{
    if (portSerie->isOpen()) portSerie->close();
    delete ui;
}

// Bouton OPEN
void MainWindow::on_ButtonOpen_2_clicked()
{
    ui->progressBar_2->setValue(10);
    QString nomport = ui->ComboComPort->currentText();
    if (nomport.isEmpty()) {
        ui->listWidget_5->addItem("Aucun port sélectionné.");
        ui->progressBar_2->setValue(0);
        return;
    }

    if (portSerie->isOpen()) portSerie->close();

    ui->progressBar_2->setValue(20);
    portSerie->setPortName(nomport);
    portSerie->setBaudRate(QSerialPort::Baud115200);
    portSerie->setDataBits(QSerialPort::Data8);
    portSerie->setParity(QSerialPort::NoParity);
    portSerie->setStopBits(QSerialPort::OneStop);
    portSerie->setFlowControl(QSerialPort::NoFlowControl);

    if (!portSerie->open(QIODevice::ReadWrite)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir " + nomport);
        ui->progressBar_2->setValue(0);
        return;
    }

    ui->listWidget_5->addItem("Port ouvert sur " + nomport);

    QThread::msleep(600);
    QByteArray discard = portSerie->readAll();
    ui->listWidget_5->addItem("Boot logs discarded: " + QString::number(discard.size()) + " bytes");

    ui->progressBar_2->setValue(60);
    QByteArray cmd = R"({"cmd":"get_config"})" "\n";
    portSerie->write(cmd);
    ui->listWidget_5->addItem("Commande envoyée : " + QString(cmd));
    ui->progressBar_2->setValue(75);

    if (portSerie->waitForReadyRead(1200)) {
        QByteArray data = portSerie->readAll();
        while (portSerie->waitForReadyRead(50)) data += portSerie->readAll();
        ui->listWidget_5->addItem("Réponse brute : " + QString::fromUtf8(data));
        traiterJsonConfig(data);
        ui->progressBar_2->setValue(100);
    } else {
        ui->listWidget_5->addItem("Aucune réponse JSON reçue du M5.");
        ui->progressBar_2->setValue(0);
    }
}

// Bouton READ
void MainWindow::on_ReadConfig_clicked()
{
    if (!portSerie->isOpen()) return;
    QByteArray cmd = R"({"cmd":"get_config"})" "\n";
    portSerie->write(cmd);
    if (portSerie->waitForReadyRead(1000)) {
        QByteArray data = portSerie->readAll();
        while (portSerie->waitForReadyRead(50)) data += portSerie->readAll();
        traiterJsonConfig(data);
    }
}

// Bouton REBOOT
void MainWindow::on_SendReboot_clicked()
{
    if (!portSerie->isOpen()) return;
    QByteArray cmd = R"({"cmd":"reboot"})" "\n";
    portSerie->write(cmd);
    ui->listWidget_5->addItem("Commande reboot envoyée");
}

// Bouton WRITE
void MainWindow::on_BouttonWrite_clicked()
{
    if (!portSerie->isOpen()) {
        ui->listWidget_5->addItem("❌ Port non ouvert !");
        return;
    }

    ui->progressBar_2->setValue(10);
    ui->listWidget_5->addItem("Préparation de la configuration...");

    QString ip = ui->LocalIpAddress->text();
    QString mask = ui->SubnetMask->text();
    QString gw = ui->GatewayIpAddress->text();
    QString dns = ui->DNSIPAddress->text();
    QString ssid = ui->SSID->text();
    QString password = ui->WEP_WPA->text();

    // Détection du mode selon le radio bouton
    QString mode = "AP";
    if (ui->INFRASTRUCTURE && ui->INFRASTRUCTURE->isChecked()) {
        mode = "STA";
    } else if (ui->SOFTAP && ui->SOFTAP->isChecked()) {
        mode = "AP";
    }

    if (ip.isEmpty() || mask.isEmpty() || gw.isEmpty() || dns.isEmpty() ||
        ssid.isEmpty() || password.isEmpty()) {
        ui->listWidget_5->addItem("❌ Tous les champs sont obligatoires !");
        ui->progressBar_2->setValue(0);
        return;
    }

    QJsonObject obj;
    obj["cmd"] = "set_config";
    obj["ip"] = ip;
    obj["mask"] = mask;
    obj["gw"] = gw;
    obj["dns"] = dns;
    obj["ssid"] = ssid;
    obj["password"] = password;
    obj["mode"] = mode;

    QJsonDocument doc(obj);
    QByteArray jsonToSend = doc.toJson(QJsonDocument::Compact) + "\n";

    ui->listWidget_5->addItem("JSON envoyé (mode " + mode + ") : " + QString(jsonToSend));
    ui->progressBar_2->setValue(50);

    portSerie->write(jsonToSend);
    portSerie->flush();
    ui->progressBar_2->setValue(70);

    if (portSerie->waitForReadyRead(2000)) {
        QByteArray rep = portSerie->readAll();
        while (portSerie->waitForReadyRead(50)) rep += portSerie->readAll();
        QString repStr = QString::fromUtf8(rep);

        ui->listWidget_5->addItem("Réponse M5 : " + repStr);

        if (repStr.contains(R"("status":"OK")")) {
            ui->listWidget_5->addItem("✅ Configuration " + mode + " appliquée !");
            ui->progressBar_2->setValue(100);
        } else {
            ui->listWidget_5->addItem("⚠️ Réponse sans OK");
            ui->progressBar_2->setValue(20);
        }
    } else {
        ui->listWidget_5->addItem("❌ Timeout");
        ui->progressBar_2->setValue(0);
    }
}

// TRAITER JSON
void MainWindow::traiterJsonConfig(const QByteArray &data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        ui->listWidget_5->addItem("Erreur JSON : " + err.errorString());
        return;
    }

    QJsonObject obj = doc.object();

    ui->LocalIpAddress->setText(obj.value("ip").toString());
    ui->SubnetMask->setText(obj.value("mask").toString());
    ui->GatewayIpAddress->setText(obj.value("gateway").toString());
    ui->DNSIPAddress->setText(obj.value("dns").toString());

    ui->SSID->setText(obj.value("ssid").toString());
    ui->WEP_WPA->setText(obj.value("password").toString());

    ui->listWidget_5->addItem("✅ Tous les champs remplis (IP, Mask, Gateway, DNS, SSID, Clé) !");
}


