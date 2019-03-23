#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMessageBox>
#include <QComboBox>
#include <QTextEdit>
#include "config.h"
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

    void on_console_line_returnPressed();

    void on_button_send_clicked();

    void on_action_settings_triggered();

    void on_gcode_edit_textChanged();

    void on_checkBox_stateChanged(int arg1);

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

    //Последовательный порт
    QSerialPort     *comPort;
    QSerialPortInfo *portInfo;
    Config          *config;
    //Главный массив с G-code
    GCode           *g_code;
    QTimer          *mainTimer;
    QGraphicsScene  *scene;

    bool previewscaling = 0;
    bool projectWorking = 0;

    int xsteps;
    int ysteps;
    int zmin;
    int zmax;
    int step_filling;
    bool xisgeneral;
};

#endif // MAINWINDOW_H
