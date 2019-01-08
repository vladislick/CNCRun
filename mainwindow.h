#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include "conffile.h"
#include <QTimer>
#include <QFileDialog>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_action_exit_triggered();

    void on_action_about_2_triggered();

    void on_action_settings_triggered();

    void on_action_open_triggered();

    void on_pushButton_clicked();

    void on_pushButton_4_clicked();

    void console();

    void on_pushButton_3_clicked();

    void on_pushButton_5_clicked();

    void timerX_slot();
    void timerY_slot();
    void timerZ_slot();

    void on_pushButton_7_pressed();

    void on_pushButton_7_released();

    void on_pushButton_13_pressed();

    void on_pushButton_13_released();

    void on_pushButton_9_pressed();

    void on_pushButton_9_released();

    void on_pushButton_11_pressed();

    void on_pushButton_11_released();

private:
    Ui::MainWindow *ui;

    void portsUpdate(QSerialPortInfo*, QComboBox*);
    bool portsAvailable(QSerialPortInfo*);
    void status(short);
    short serialWrite(QSerialPort*, const char*, int);

    //Координаты
    short axisX = 0;
    short axisY = 0;
    short axisZ = 0;
    //Ускорение
    short dx = 0;
    short dy = 0;
    short dz = 0;

    //Последовательный порт
    QSerialPort     *comPort;
    QSerialPortInfo *portInfo;
    QFileDialog     *fileDialog;
    ConfFile        *file;
    QTimer          *timerX;
    QTimer          *timerY;
    QTimer          *timerZ;
    QString         *gcode;
};

#endif // MAINWINDOW_H
