#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QComboBox>
#include <QTextEdit>
#include "conffile.h"
#include "gcode.h"
#include <QTimer>
#include <QFileDialog>
#include <QImage>
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

    void on_button_clear_clicked();

    void on_button_start_clicked();

    void on_button_com_clicked();

    void on_action_open_triggered();

    void on_button_home_clicked();

    void data_exchange_timer();

    void on_action_about_triggered();

private:
    Ui::MainWindow *ui;

    ///Отправить текст на консоль вывода
    void    consoleWrite(QString, QTextEdit*);
    ///Отправить текст в порт
    short   serialWrite(QSerialPort*, const char*, int);
    ///Обновляет данные о портах
    void    serialUpdate(QSerialPortInfo*, QComboBox*);
    ///Возвращает количество доступных портов
    int     serialAvailable(QSerialPortInfo*);
    ///Обрабатывает открытие файла
    void    fileOpen(QString);
    ///Обновляет интерфейс программы
    void    uiUpdate();

    void    start();

    //Последовательный порт
    QSerialPort     *comPort;
    QSerialPortInfo *portInfo;
    ConfFile        *conffile;
    //Главный массив с G-code
    GCode           *g_code;
    QTimer          *mainTimer;

    bool projectWorking = 0;

    int xsteps;
    int ysteps;
    int step_filling;
    bool xisgeneral;
};

#endif // MAINWINDOW_H
