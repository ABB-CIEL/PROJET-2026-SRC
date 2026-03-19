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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_ButtonOpen_2_clicked();
    void on_ButtonReadSerial_2_clicked();
    void on_Buttonreboot_2_clicked();

    void on_ReadConfig_clicked();
    void on_SendReboot_clicked();

    void on_progressBar_2_valueChanged(int value);
    void on_verticalScrollBar_3_actionTriggered(int action);
    void on_horizontalScrollBar_5_actionTriggered(int action);
    void on_ViewCommentSerial_checkStateChanged(const Qt::CheckState &state);
    void on_BuutonSerachAll_clicked();
    void on_BouttonWrite_clicked();


private:
    void traiterJsonConfig(const QByteArray &data);

    Ui::MainWindow *ui;
    QSerialPort *portSerie;
};

#endif // MAINWINDOW_H
