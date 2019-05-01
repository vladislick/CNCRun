#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGuiApplication>
#include <QGraphicsView>
#include <QMessageBox>
#include <QComboBox>
#include <QTextEdit>
#include <QColor>
#include "config.h"
#include "gcode.h"
#include <QTimer>
#include <QtGui>
#include <QFileDialog>
#include <QImage>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

//Время, через которое будет перерисовываться картинка
#define PREVIEW_TIMER_DELAY 200

//Время, через которое будет обновляться информация о портах
#define IO_TIMER_DELAY      2000


#define MAIN_TIMER_DELAY    10

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

    void preview_update_timer();

    void io_update_timer();

    void on_action_about_triggered();

    void on_console_line_returnPressed();

    void on_button_send_clicked();

    void on_action_settings_triggered();

    void on_gcode_edit_textChanged();

    void on_checkBox_stateChanged(int arg1);

    void on_penSize_valueChanged(int value);

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
    ///Начать выполнение G-code
    void    start();
    ///Рисует картинку предпросмотра
    void    previewRender(QString, QGraphicsScene*, int, int, bool, int);
    ///Читает все настройки программы
    void    settingsRead();

    QSerialPort     *comPort;
    QSerialPortInfo *portInfo;
    QGraphicsScene  *scene;
    Config          *config;
    GCode           *g_code;
    QTimer          *mainTimer;
    QTimer          *previewTimer;
    QTimer          *ioTimer;
    QColor          *penColor;
    QColor          *penColorLight;

    bool previewscaling = 1;
    bool projectWorking = 0;

    /* Аналоги параметров в найстройках */
    int     xsteps;
    int     ysteps;
    int     zmin;
    int     zmax;
    int     step_filling;
    bool    xisgeneral;
    bool    changeaxis;
    bool    dirchange;
    int     timeout;
    QString answer;

};

#endif // MAINWINDOW_H
