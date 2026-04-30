/**
 * @file    mainwindow.cpp
 * @brief   Implémentation de la classe MainWindow.
 * @date    2026
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QThread>

/**
 * @brief Constructeur : initialise l'IHM, instancie le port série et liste les ports disponibles.
 */
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

/**
 * @brief Destructeur : ferme proprement le port série et libère l'IHM.
 */
MainWindow::~MainWindow()
{
    if (portSerie->isOpen()) portSerie->close();
    delete ui;
}

/**
 * @brief Ouvre le port série sélectionné, configure la liaison et purge les logs de boot.
 */
//  OPEN (seulement connexion + vidage)
void MainWindow::on_ButtonOpen_2_clicked()
{
    ui->progressBar_2->setValue(10);
    QString nomport = ui->ComboComPort->currentText();
    if (nomport.isEmpty()) {
        ui->listWidget_5->addItem("❌ Aucun port sélectionné.");
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

    ui->listWidget_5->addItem("✅ Port ouvert sur " + nomport);

    // VIDAGE ULTRA-RENFORCÉ
    QThread::msleep(1200);
    QByteArray discard;
    for (int i = 0; i < 15; i++) {
        QThread::msleep(80);
        discard += portSerie->readAll();
    }
    ui->listWidget_5->addItem("Boot logs discarded: " + QString::number(discard.size()) + " bytes");

    ui->listWidget_5->addItem("✅ Connexion établie avec le M5 !");
    ui->progressBar_2->setValue(100);
}

/**
 * @brief Envoie la commande get_config et exploite la réponse JSON pour remplir l'IHM.
 */
//  READ (envoie get_config)
void MainWindow::on_ReadConfig_clicked()
{
    if (!portSerie->isOpen()) {
        ui->listWidget_5->addItem("❌ Port non ouvert !");
        return;
    }

    ui->progressBar_2->setValue(10);
    QByteArray cmd = R"({"cmd":"get_config"})" "\n";
    portSerie->write(cmd);
    ui->listWidget_5->addItem("Commande envoyée : get_config");
    ui->progressBar_2->setValue(40);

    if (portSerie->waitForReadyRead(10000)) {
        QByteArray data = portSerie->readAll();
        while (portSerie->waitForReadyRead(80)) data += portSerie->readAll();

        ui->listWidget_5->addItem("Réponse brute : " + QString::fromUtf8(data));
        traiterJsonConfig(data);
        ui->progressBar_2->setValue(100);
    } else {
        ui->listWidget_5->addItem("❌ Timeout - Aucune réponse du M5");
        ui->progressBar_2->setValue(0);
    }
}

/**
 * @brief Envoie la commande JSON reboot au M5.
 */
//  REBOOT =
void MainWindow::on_SendReboot_clicked()
{
    if (!portSerie->isOpen()) return;
    QByteArray cmd = R"({"cmd":"reboot"})" "\n";
    portSerie->write(cmd);
    ui->listWidget_5->addItem("Commande reboot envoyée");
}

//  SLOTS VIDES
/** @brief Slot réservé. */
void MainWindow::on_ButtonReadSerial_2_clicked() { }
/** @brief Slot réservé. */
void MainWindow::on_Buttonreboot_2_clicked() { }
/** @brief Slot réservé. */
void MainWindow::on_progressBar_2_valueChanged(int value) { Q_UNUSED(value); }
/** @brief Slot réservé. */
void MainWindow::on_verticalScrollBar_3_actionTriggered(int action) { Q_UNUSED(action); }
/** @brief Slot réservé. */
void MainWindow::on_horizontalScrollBar_5_actionTriggered(int action) { Q_UNUSED(action); }
/** @brief Slot réservé. */
void MainWindow::on_ViewCommentSerial_checkStateChanged(const Qt::CheckState &state) { Q_UNUSED(state); }
/** @brief Slot réservé. */
void MainWindow::on_BuutonSerachAll_clicked() { }

// ===================== TRAITER JSON =====================
/**
 * @brief  Parse la trame JSON et met à jour tous les champs de l'IHM.
 * @param  data Trame brute reçue sur le port série.
 */
void MainWindow::traiterJsonConfig(const QByteArray &data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        ui->listWidget_5->addItem("Erreur JSON : " + err.errorString());
        return;
    }

    QJsonObject obj = doc.object();
    QString mode = obj.value("mode").toString();

    ui->LocalIpAddress->setText(obj.value("ip").toString());
    ui->SubnetMask->setText(obj.value("mask").toString());
    ui->GatewayIpAddress->setText(obj.value("gateway").toString());
    ui->DNSIPAddress->setText(obj.value("dns").toString());

    if (mode == "STA") {
        ui->SSID->setText(obj.value("sta_ssid").toString());
        ui->WEP_WPA->setText(obj.value("sta_password").toString());
    } else {
        ui->SSID->setText(obj.value("ap_ssid").toString());
        ui->WEP_WPA->setText(obj.value("ap_password").toString());
    }

    int baud = obj.value("baud_rate").toInt(9600);
    ui->Baudrate->setCurrentText(QString::number(baud));

    QString parity = obj.value("parity").toString();
    if (parity == "N") parity = "None";
    else if (parity == "E") parity = "Even";
    else if (parity == "O") parity = "Odd";
    ui->Parity->setCurrentText(parity);

    ui->DataBits->setCurrentText(QString::number(obj.value("data_bits").toInt(8)));
    ui->StopBit->setCurrentText(QString::number(obj.value("stop_bits").toInt(1)));
    ui->SerialType->setCurrentText(obj.value("serial_type").toString("RS232"));

    ui->listWidget_5->addItem("✅ Tous les champs + Serial Port remplis !");
}

// ===================== WRITE (inchangé) =====================
/**
 * @brief Construit la trame JSON set_config à partir de l'IHM et l'envoie au M5.
 */
void MainWindow::on_BouttonWrite_clicked()
{
    if (!portSerie->isOpen()) {
        ui->listWidget_5->addItem("❌ Port non ouvert !");
        return;
    }

    ui->progressBar_2->setValue(10);
    ui->listWidget_5->addItem("Préparation de la configuration...");

    QString ip       = ui->LocalIpAddress->text();
    QString mask     = ui->SubnetMask->text();
    QString gw       = ui->GatewayIpAddress->text();
    QString dns      = ui->DNSIPAddress->text();
    QString ssid     = ui->SSID->text();
    QString password = ui->WEP_WPA->text();

    QString mode = "AP";
    if (ui->INFRASTRUCTURE && ui->INFRASTRUCTURE->isChecked()) mode = "STA";
    else if (ui->SOFTAP && ui->SOFTAP->isChecked()) mode = "AP";

    if (ssid.isEmpty() || password.isEmpty()) {
        ui->listWidget_5->addItem("❌ SSID et clé sont obligatoires !");
        ui->progressBar_2->setValue(0);
        return;
    }

    QJsonObject obj;
    obj["cmd"]      = "set_config";
    obj["ip"]       = ip;
    obj["mask"]     = mask;
    obj["gw"]       = gw;
    obj["dns"]      = dns;
    obj["ssid"]     = ssid;
    obj["password"] = password;
    obj["mode"]     = mode;

    QJsonDocument doc(obj);
    QByteArray jsonToSend = doc.toJson(QJsonDocument::Compact) + "\n";

    ui->listWidget_5->addItem("JSON envoyé (mode " + mode + ") : " + QString(jsonToSend));
    ui->progressBar_2->setValue(50);

    portSerie->write(jsonToSend);
    portSerie->flush();
    ui->progressBar_2->setValue(70);

    if (portSerie->waitForReadyRead(1500)) {
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

/** @brief Slot de changement du type de liaison série. */
void MainWindow::on_SerialType_currentIndexChanged(int index)
{

}
