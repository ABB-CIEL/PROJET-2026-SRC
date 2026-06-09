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
 * @brief Constructeur : initialise l'IHM, instancie le port série, liste les ports
 *        disponibles et remplit les combos de l'onglet Serial Port avec les valeurs standards.
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

    // === Remplissage des combos Serial Port ===
    ui->Baudrate->clear();
    ui->Baudrate->addItems({"1200","2400","4800","9600","19200","38400","57600","115200"});

    ui->Parity->clear();
    ui->Parity->addItems({"None","Even","Odd"});

    ui->DataBits->clear();
    ui->DataBits->addItems({"5","6","7","8"});

    ui->StopBit->clear();
    ui->StopBit->addItems({"1","2"});

    ui->SerialType->clear();
    ui->SerialType->addItems({"RS232","RS485"});

    // === Combo Product (vide tant que pas connecté) ===
    ui->ComboProduct->clear();

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
 *        Affiche aussi le nom de l'appareil connecté dans le combo Product.
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

    // === Affichage du nom de l'appareil dans le combo Product ===
    ui->ComboProduct->clear();
    ui->ComboProduct->addItem("M5 STACK CODE PRO");
    ui->ComboProduct->setCurrentText("M5 STACK CODE PRO");

    ui->progressBar_2->setValue(100);
}

/**
 * @brief Envoie la commande get_config et exploite la réponse JSON pour remplir l'onglet Network.
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

// ===================== READ SERIAL (onglet Serial Port) =====================
/**
 * @brief Envoie get_config et remplit uniquement les champs de l'onglet Serial Port.
 */
void MainWindow::on_ButtonReadSerial_2_clicked()
{
    if (!portSerie->isOpen()) {
        ui->listWidget_5->addItem("❌ Port non ouvert !");
        return;
    }

    ui->progressBar_2->setValue(10);
    QByteArray cmd = R"({"cmd":"get_config"})" "\n";
    portSerie->write(cmd);
    ui->listWidget_5->addItem("Commande envoyée : get_config (Serial)");
    ui->progressBar_2->setValue(40);

    if (portSerie->waitForReadyRead(10000)) {
        QByteArray data = portSerie->readAll();
        while (portSerie->waitForReadyRead(80)) data += portSerie->readAll();

        ui->listWidget_5->addItem("Réponse brute : " + QString::fromUtf8(data));
        traiterJsonSerial(data);
        ui->progressBar_2->setValue(100);
    } else {
        ui->listWidget_5->addItem("❌ Timeout - Aucune réponse du M5");
        ui->progressBar_2->setValue(0);
    }
}

// ===================== REBOOT SERIAL =====================
/**
 * @brief Envoie un reboot depuis l'onglet Serial Port.
 */
void MainWindow::on_Buttonreboot_2_clicked()
{
    if (!portSerie->isOpen()) return;
    QByteArray cmd = R"({"cmd":"reboot"})" "\n";
    portSerie->write(cmd);
    ui->listWidget_5->addItem("Commande reboot envoyée (Serial)");
}

// ===================== MODE IP STATIQUE (Soft Ap) =====================
/**
 * @brief Active le mode IP statique : seul Local IP est modifiable, on force le mode AP.
 */
void MainWindow::on_SoftApIPAddress_clicked()
{
    // Local IP éditable, autres champs verrouillés
    ui->LocalIpAddress->setReadOnly(false);
    ui->SubnetMask->setReadOnly(true);
    ui->GatewayIpAddress->setReadOnly(true);
    ui->DNSIPAddress->setReadOnly(true);

    // Force le mode AP (Soft Ap)
    if (ui->SOFTAP) ui->SOFTAP->setChecked(true);

    ui->listWidget_5->addItem("Mode Soft Ap : seul Local IP est modifiable");
}

// ===================== MODE DHCP =====================
/**
 * @brief Active le mode DHCP : tous les champs IP passent à 0.0.0.0 et sont verrouillés,
 *        on force le mode STA pour que le réseau attribue une IP.
 */
void MainWindow::on_DHCP_clicked()
{
    // Tous les champs IP à 0.0.0.0
    ui->LocalIpAddress->setText("0.0.0.0");
    ui->SubnetMask->setText("0.0.0.0");
    ui->GatewayIpAddress->setText("0.0.0.0");
    ui->DNSIPAddress->setText("0.0.0.0");

    // Tous les champs verrouillés (l'IP sera attribuée par le DHCP)
    ui->LocalIpAddress->setReadOnly(true);
    ui->SubnetMask->setReadOnly(true);
    ui->GatewayIpAddress->setReadOnly(true);
    ui->DNSIPAddress->setReadOnly(true);

    // Force le mode STA (Infrastructure) pour que le réseau attribue une IP
    if (ui->INFRASTRUCTURE) ui->INFRASTRUCTURE->setChecked(true);

    ui->listWidget_5->addItem("Mode DHCP : IP V4 mises à 0.0.0.0, mode STA");
}

//  SLOTS VIDES
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

// ===================== TRAITER JSON (Network) =====================
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

// ===================== TRAITER JSON (Serial Port uniquement) =====================
/**
 * @brief  Parse la trame JSON et remplit uniquement les champs de l'onglet Serial Port.
 * @param  data Trame brute reçue sur le port série.
 */
void MainWindow::traiterJsonSerial(const QByteArray &data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        ui->listWidget_5->addItem("Erreur JSON : " + err.errorString());
        return;
    }

    QJsonObject obj = doc.object();

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

    ui->listWidget_5->addItem("✅ Champs Serial Port remplis !");
}

// ===================== WRITE NETWORK =====================
/**
 * @brief Construit la trame JSON set_config (Network) et l'envoie au M5.
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

// ===================== WRITE SERIAL (onglet Serial Port) =====================
/**
 * @brief Construit la trame JSON set_config (Serial Port) et l'envoie au M5.
 */
void MainWindow::on_BouttonWriteSerial_clicked()
{
    if (!portSerie->isOpen()) {
        ui->listWidget_5->addItem("❌ Port non ouvert !");
        return;
    }

    ui->progressBar_2->setValue(10);
    ui->listWidget_5->addItem("Préparation de la configuration Serial Port...");

    int     baudRate   = ui->Baudrate->currentText().toInt();
    int     dataBits   = ui->DataBits->currentText().toInt();
    int     stopBits   = ui->StopBit->currentText().toInt();
    QString serialType = ui->SerialType->currentText();

    // Conversion parity (texte IHM -> code attendu par le M5)
    QString parityUi = ui->Parity->currentText();
    QString parity   = "N";
    if (parityUi == "Even")      parity = "E";
    else if (parityUi == "Odd")  parity = "O";
    else                          parity = "N";

    if (baudRate <= 0 || dataBits <= 0 || stopBits <= 0) {
        ui->listWidget_5->addItem("❌ Paramètres Serial invalides !");
        ui->progressBar_2->setValue(0);
        return;
    }

    QJsonObject obj;
    obj["cmd"]         = "set_config";
    obj["baud_rate"]   = baudRate;
    obj["parity"]      = parity;
    obj["data_bits"]   = dataBits;
    obj["stop_bits"]   = stopBits;
    obj["serial_type"] = serialType;

    QJsonDocument doc(obj);
    QByteArray jsonToSend = doc.toJson(QJsonDocument::Compact) + "\n";

    ui->listWidget_5->addItem("JSON envoyé (Serial) : " + QString(jsonToSend));
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
            ui->listWidget_5->addItem("✅ Configuration Serial Port appliquée !");
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
