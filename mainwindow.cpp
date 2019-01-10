#include "mainwindow.h"
#include "aboutapp.h"
#include "settings.h"
#include "ui_mainwindow.h"

MainWindow::~MainWindow()
{
    delete ui;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    conffile = new ConfFile("settings.conf");

    /* Определяем последовательный порт */
    comPort = new QSerialPort();
    portInfo = new QSerialPortInfo();
    comPort->setBaudRate(QSerialPort::Baud9600);
    comPort->setFlowControl(QSerialPort::NoFlowControl);
    comPort->setParity(QSerialPort::NoParity);
    comPort->setDataBits(QSerialPort::Data8);
    comPort->setStopBits(QSerialPort::OneStop);

    mainTimer = new QTimer();

    /* Подключаем сигналы и слоты */
    connect(mainTimer, SIGNAL(timeout()), this, SLOT(data_exchange_timer()));

    /* Выполняем начальный функционал */
    consoleWrite("Welcome!", ui->console);
    uiUpdate();
}

///Отправить текст на консоль вывода
void MainWindow::consoleWrite(QString str, QTextEdit *textedit) {
    textedit->append(str);
}

///Отправить текст в порт
short MainWindow::serialWrite(QSerialPort* serial, const char* str, int length) {
    char err;
    if (serial->isOpen()) {
        err = serial->error();
        if (!err) {
            serial->write(str, length);
            return 1;
        }
        else if (err == QSerialPort::ResourceError) {
            serial->close();
            return -1;
        }
        else {
            return -2;
        }
    } else return 0;
}

///Обновляет данные о портах
void MainWindow::serialUpdate(QSerialPortInfo* port, QComboBox* box) {
    box->clear();
       if (serialAvailable(port)) {
           for (char i = 0; i < port->availablePorts().size(); i++) {
               box->addItem(port->availablePorts()[i].portName());
           }
       } else {
           box->addItem("Не найдено");
       }
}

///Возвращает количество доступных портов
int MainWindow::serialAvailable(QSerialPortInfo* port) {
    return port->availablePorts().size();
}

///Обрабатывает открытие файла
void MainWindow::fileOpen(QString path) {
    QFile file(path);

    /* Если это файл с G-code */
    if (path.indexOf(".txt") > 0) {
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            g_code = QString::fromUtf8(file.readAll().toStdString().c_str());
            ui->gcode_edit->setText(g_code);
        } else {
            QMessageBox box(QMessageBox::Critical, "Невозможно открыть файл", "Не удалось открыть выбранный файл");
            box.exec();
        }
    }
    /* Если это картинка */
    else if (path.indexOf(".png") > 0 || path.indexOf(".jpg") > 0) {
        QImage img(path);
        QString filename;
        QColor pixelColor;
        bool pixels[240][160];
        int index;

        /* Получаем имя файла */
        if (path.lastIndexOf('/') > 0)
            index = path.lastIndexOf('/'); //Если Unix-система
        else
            index = path.lastIndexOf('\\'); //Если Windows
        filename = path;
        filename.remove(index);

        if (img.width() == 240 && img.height() == 160) {
            /* Сканируем по X */
            for (int x = 0; x < img.width(); x++) {
                /* Сканируем по Y */
                for (int y = 0; y < img.height(); y++) {
                    pixelColor = img.pixelColor(x, y);
                    if (pixelColor.red() + pixelColor.red() + pixelColor.red() > 381) {
                        pixels[x][y] = 0; //Если это светлый цвет
                    } else {
                        pixels[x][y] = 1; //Если это тёмный цвет
                    }
                }
            }

            /* Генерируем G-code */
            g_code.clear();
            g_code += "G00 Z170\nG00 X00 Y00\n"; //Встаём на нулевую координату

            bool direction = 1;
            bool gap_now = 0;
            int  gap = 0;

            //Проходим по каждой строчке
            for (int y = 0; y < img.height(); y++) {
                gap_now = 0;
                //Если вправо
                if (direction) {
                    for (int x = 0; x < img.width(); x++) {
                        //Если нужно рисовать пиксель
                        if (pixels[x][y]) {
                            if (!gap_now) {
                                gap = x;
                                gap_now = 1;
                            }
                            if (x == (img.width() - 1)) {
                                g_code += "G00 X" + QString::number(gap) + " Y" + QString::number(y) + '\n';
                                g_code += "G00 Z130\n";
                                g_code += "G00 X" + QString::number(x) + '\n';
                                g_code += "G00 Z170\n";
                            }
                        }
                        //Если рисовать пиксель не нужно
                        else {
                            if (gap_now) {
                                gap_now = 0;
                                g_code += "G00 X" + QString::number(gap) + " Y" + QString::number(y) + '\n';
                                g_code += "G00 Z130\n";
                                g_code += "G00 X" + QString::number(x - 1) + '\n';
                                g_code += "G00 Z170\n";
                            }
                        }
                    }
                }
            }

            g_code += "G00 Z170\n";
            g_code += "G00 X00 Y160\n";
            ui->gcode_edit->setText(g_code);
        } else {
            QMessageBox box(QMessageBox::Critical, "Изображение не подходит", "Выбранное изображение не подходит для преобразования в G-code\nПожалуйста, выберите изображение с размером 240x160 пикселей");
            box.exec();
        }
    }
}

///Обновляет интерфейс программы
void MainWindow::uiUpdate() {
    bool isOpen = comPort->isOpen();
    ui->button_start->setEnabled(isOpen);
    ui->button_home->setEnabled(isOpen);
    ui->button_up->setEnabled(isOpen);
    ui->button_down->setEnabled(isOpen);
    ui->button_forward->setEnabled(isOpen);
    ui->button_backward->setEnabled(isOpen);
    ui->button_left->setEnabled(isOpen);
    ui->button_right->setEnabled(isOpen);
    ui->console_line->setEnabled(isOpen);
    ui->button_send->setEnabled(isOpen);
    if (isOpen) ui->button_com->setText("Отключить");
    else {
        serialUpdate(portInfo, ui->comboBox);
        if (serialAvailable(portInfo)) {
            ui->button_com->setText("Подключить");
        } else {
            ui->button_com->setText("Обновить");
        }
    }
}

void MainWindow::on_action_exit_triggered()
{
    this->close();
}

void MainWindow::on_button_clear_clicked()
{
    ui->console->clear();
}

void MainWindow::data_exchange_timer() {
    static int      index = 0;
    static int      time;
    static QString  command;
    static QByteArray data;
    static bool     issended = 0;

    /* Если уже идёт отправка данных */
    if (index > 0 && issended) {
        data = comPort->readAll();
        if (data != "y") {
            if (time > 0) {
                time--;
                return;
            } else {
                mainTimer->stop();
                consoleWrite("Connection TIMEOUT, Sorry [SYSTEM]", ui->console);
                on_button_start_clicked();
                issended = 0;
                index = 0;
                return;
            }
        }
    } else comPort->clear(); //Очищаем данные с порта

    /* Если команда уже была отправлена */
    if (issended) {
        issended = 0;
        consoleWrite("OK", ui->console);
    }

    /* Чтение команды из текстового поля */
    if (g_code[index] == '\n') {
        /* Отправляем команду */
        if (serialWrite(comPort, command.toStdString().c_str(), command.length() + 1)) {
            consoleWrite("Sended: " + command + " [SYSTEM]", ui->console);
        } else { //Если не удалось отправить команду
            consoleWrite("Connection BROKEN, Sorry [SYSTEM]", ui->console);
            on_button_start_clicked();
            mainTimer->stop();
            issended = 0;
            index = 0;
            return;
        }
        issended = 1;
        time = 1200;
        command.clear();
    } else {
        command += g_code[index];
    }

    /* Обнуляем index, если прошлись по всем символам */
    if (index < g_code.length()) index++;
    else {
        index = 0;
        issended = 0;
        mainTimer->stop();
        consoleWrite("Completed with no errors! [SYSTEM]", ui->console);
        on_button_start_clicked();
    }
}

void MainWindow::on_button_start_clicked()
{
    uiUpdate();
    if (ui->button_start->text() == "Начать") {
        g_code = ui->gcode_edit->toPlainText();
        ui->button_start->setText("Остановить");
        ui->button_home->setEnabled(0);
        ui->button_up->setEnabled(0);
        ui->button_down->setEnabled(0);
        ui->button_forward->setEnabled(0);
        ui->button_backward->setEnabled(0);
        ui->button_left->setEnabled(0);
        ui->button_right->setEnabled(0);
        ui->console_line->setEnabled(0);
        ui->button_send->setEnabled(0);
        consoleWrite("Project is started! [SYSTEM]", ui->console);
        mainTimer->start(5);
    } else {
        ui->button_start->setText("Начать");
        uiUpdate();
    }
}

void MainWindow::on_button_com_clicked()
{
    if (ui->button_com->text() == "Подключить") {
        comPort->setPortName(ui->comboBox->currentText());
        if (comPort->open(QIODevice::ReadWrite)) {
            consoleWrite("Connected to " + ui->comboBox->currentText() + " [SYSTEM]", ui->console);
        } else {
            consoleWrite("Cannot connect to " + ui->comboBox->currentText() + " [SYSTEM]", ui->console);
        }
    } else if (ui->button_com->text() == "Отключить") {
        comPort->close();
    }
    uiUpdate();
}

void MainWindow::on_action_open_triggered()
{
    QFileDialog fileDialog;
    QString str = fileDialog.getOpenFileName(this, tr("Открыть"), "", tr("Изображение или G-code (*.jpg *.png *.txt)"));
    if (str.size() != 0) fileOpen(str);
}

void MainWindow::on_button_home_clicked()
{
    if (serialWrite(comPort, "G00 X00 Y00 Z170", 17) < 0) {
        uiUpdate();
        consoleWrite("Connection BROKEN, Sorry [SYSTEM]", ui->console);
    } else consoleWrite("Moving to HOME position [USER]", ui->console);
}

void MainWindow::on_action_about_triggered()
{
    AboutApp about;
    about.setModal(true);
    about.exec();
}
