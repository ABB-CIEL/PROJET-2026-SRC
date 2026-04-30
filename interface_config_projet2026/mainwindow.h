/**
 * @file    mainwindow.h
 * @brief   Déclaration de la classe MainWindow (fenêtre principale).
 * @date    2026
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
 * @class   MainWindow
 * @brief   Fenêtre principale : ouverture du port série, lecture/écriture
 *          de la configuration du M5 et envoi de commandes.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief  Constructeur.
     * @param  parent Widget parent.
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief  Destructeur : ferme le port série s'il est ouvert.
     */
    ~MainWindow();

private slots:
    /** @brief  Ouvre le port série sélectionné et purge les logs de boot du M5. */
    void on_ButtonOpen_2_clicked();
    /** @brief  Slot réservé (lecture série brute). */
    void on_ButtonReadSerial_2_clicked();
    /** @brief  Slot réservé (reboot bouton secondaire). */
    void on_Buttonreboot_2_clicked();

    /** @brief  Envoie la commande JSON get_config et remplit l'IHM avec la réponse. */
    void on_ReadConfig_clicked();
    /** @brief  Envoie la commande JSON reboot au M5. */
    void on_SendReboot_clicked();

    /** @brief  Slot lié à la barre de progression. */
    void on_progressBar_2_valueChanged(int value);
    /** @brief  Slot lié à la barre de défilement verticale. */
    void on_verticalScrollBar_3_actionTriggered(int action);
    /** @brief  Slot lié à la barre de défilement horizontale. */
    void on_horizontalScrollBar_5_actionTriggered(int action);
    /** @brief  Slot d'activation/désactivation des commentaires série. */
    void on_ViewCommentSerial_checkStateChanged(const Qt::CheckState &state);
    /** @brief  Slot du bouton de recherche globale. */
    void on_BuutonSerachAll_clicked();
    /** @brief  Construit et envoie la commande JSON set_config au M5. */
    void on_BouttonWrite_clicked();


    /** @brief  Slot de changement du type de liaison série. */
    void on_SerialType_currentIndexChanged(int index);

private:
    /**
     * @brief  Analyse la réponse JSON reçue du M5 et met à jour les champs de l'IHM.
     * @param  data Trame brute reçue sur le port série.
     */
    void traiterJsonConfig(const QByteArray &data);

    Ui::MainWindow *ui;        /**< Pointeur vers l'interface graphique. */
    QSerialPort *portSerie;    /**< Port série utilisé pour communiquer avec le M5. */
};

#endif // MAINWINDOW_H
