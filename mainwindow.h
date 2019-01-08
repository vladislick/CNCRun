#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QComboBox>
#include <QTextEdit>
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

    void on_button_clear_clicked();

    void on_button_start_clicked();

    void on_button_com_clicked();

    void on_action_open_triggered();

private:
    Ui::MainWindow *ui;

    ///Отправить текст на консоль вывода
    void consoleWrite(QString, QTextEdit*);
    ///Отправить текст в порт
    short serialWrite(QSerialPort*, const char*, int);
    ///Обновляет данные о портах
    void serialUpdate(QSerialPortInfo*, QComboBox*);
    ///Возвращает количество доступных портов
    int serialAvailable(QSerialPortInfo*);
    ///Обрабатывает открытие файла
    void fileOpen(QString);
    ///Старт работы станка
    void start();

    //Последовательный порт
    QSerialPort     *comPort;
    QSerialPortInfo *portInfo;
    ConfFile        *conffile;
    //Главный массив с G-code
    QString      gcode;
};

#endif // MAINWINDOW_H
