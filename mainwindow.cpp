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

    connectionTimer = new QTimer(this);

    /* Подключаем сигналы и слоты */
    connect(connectionTimer, SIGNAL(timeout()), this, SLOT(connection_timer()));

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
    if (path.indexOf(".txt")) {
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            g_code = QString::fromUtf8(file.readAll().toStdString().c_str());
            ui->gcode_edit->setText(g_code);
        } else {
            QMessageBox box(QMessageBox::Critical, "Невозможно открыть файл", "Не удалось открыть выбранный файл");
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

void MainWindow::connection_timer() {
    connectionTimeout = 1;
}

void MainWindow::start() {
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

    QString command;
    QByteArray received = comPort->readAll();
    received.clear();

    for (int i = 0; i < g_code.length(); i++) {
       if (g_code[i] == '\n' || i == (g_code.length() - 1)) {
            if (i == (g_code.length() - 1)) command += g_code[i];

            /* Отправляем команду на станок */
            if (serialWrite(comPort, command.toStdString().c_str(), command.size() + 1) < 0) {
                consoleWrite("Connection ERROR [SYSTEM]", ui->console);
                break;
            } else consoleWrite("Sended: " + command, ui->console);

            /* Ждём подтверждения выполнения */
            connectionTimeout = 0;
            connectionTimer->start(1);
            while (!connectionTimeout) {

            }
            if (connectionTimeout) {
                consoleWrite("Connection TIMEOUT [SYSTEM]", ui->console);
                break;
            }
            connectionTimer->stop();

            /* Подготовка к следующей команде */
            received.clear();
            command.clear();
        } else {
            command += g_code[i];
        }
    }

    consoleWrite("Comleted!", ui->console);
}

void MainWindow::on_button_start_clicked()
{
    if (ui->button_start->text() == "Начать") {
        start();
    }

    ui->button_start->setText("Начать");
    uiUpdate();
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
    if (serialWrite(comPort, "G00 X00 Y00 Z130", 17) < 0) {
        uiUpdate();
        consoleWrite("Connection ERROR [SYSTEM]", ui->console);
    } else consoleWrite("Moving to HOME [USER]", ui->console);
}
