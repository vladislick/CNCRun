#include "mainwindow.h"
#include "aboutapp.h"
#include "settings.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->textEdit->append("Welcome to CNCRun :D");
    timerX = new QTimer();
    timerY = new QTimer();
    timerZ = new QTimer();
    gcode = new QString;

    fileDialog = new QFileDialog();
    file = new ConfFile("settings.conf");
    if (file->read()) {
        ui->textEdit->append("Configuration file OK (System)");
    } else {
        ui->textEdit->append("Configuration file is missing (System)");
        if (file->create()) {
            ui->textEdit->append("Configuration file is created (System)");
            ui->textEdit->append("Please, check your configuration (System)");
        } else ui->textEdit->append("Can't create configuration file! (System)");
    }

    comPort = new QSerialPort();
    portInfo = new QSerialPortInfo();

    comPort->setBaudRate(QSerialPort::Baud9600);
    comPort->setFlowControl(QSerialPort::NoFlowControl);
    comPort->setParity(QSerialPort::NoParity);
    comPort->setDataBits(QSerialPort::Data8);
    comPort->setStopBits(QSerialPort::OneStop);

    on_pushButton_clicked();

    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(console()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(console()));
    connect(timerX, SIGNAL(timeout()), this, SLOT(timerX_slot()));
    connect(timerY, SIGNAL(timeout()), this, SLOT(timerY_slot()));
    connect(timerZ, SIGNAL(timeout()), this, SLOT(timerZ_slot()));
}

///Прерывание по таймеру
void MainWindow::timerX_slot() {
    if (dx > 0) {
        axisX += dx;
        if (axisX > 239) axisX = 240;
    } else if (dx < 0){
        axisX += dx;
        if (axisX < 0) axisX = 0;
    }
    *gcode = "G00 X" + QString::number(axisX);
    serialWrite(comPort, gcode->toStdString().c_str(), gcode->size() + 1);
    ui->textEdit->append(gcode->toStdString().c_str());
}

///Прерывание по таймеру
void MainWindow::timerY_slot() {
    if (dy > 0) {
        axisY += dy;
        if (axisY > 159) axisY = 160;
    } else if (dy < 0){
        axisY += dy;
        if (axisY < 0) axisY = 0;
    }
    *gcode = "G00 Y" + QString::number(axisY);
    serialWrite(comPort, gcode->toStdString().c_str(), gcode->size() + 1);
    ui->textEdit->append(gcode->toStdString().c_str());
}

///Прерывание по таймеру
void MainWindow::timerZ_slot() {

}

///Отправка данных по COM порту
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

void MainWindow::portsUpdate(QSerialPortInfo* port, QComboBox* box) {
    box->clear();
    if (portsAvailable(port)) {
        for (char i = 0; i < port->availablePorts().size(); i++) {
            box->addItem(port->availablePorts()[i].portName());
        }
    } else {
        box->addItem("Не найдено");
    }
}

bool MainWindow::portsAvailable(QSerialPortInfo* port) {
    return port->availablePorts().size();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::console() {
    short out = serialWrite(comPort, ui->lineEdit->text().toStdString().c_str(), ui->lineEdit->text().size() + 1);
    if (out == 1) {
        ui->textEdit->append(ui->lineEdit->text() + " (user)");
    }
    else if (out == -1 || out == -2) {
        ui->textEdit->append("Connection BROKEN (System)");
        status(0);
    }
}

void MainWindow::on_action_about_2_triggered()
{
    AboutApp about;
    about.setModal(true);
    about.exec();
}

void MainWindow::on_action_settings_triggered()
{
    Settings settings;
    settings.setModal(true);
    settings.exec();
}

void MainWindow::on_action_open_triggered()
{
    QString str = fileDialog->getOpenFileName(this, tr("Открыть"), "", tr("Изображения (*.jpg *.png)"));;
}

void MainWindow::status(short type) {
    ui->pushButton_3->setEnabled(type);
    ui->pushButton_4->setEnabled(type);
    ui->lineEdit->setEnabled(type);
    ui->pushButton_2->setEnabled(type);
    ui->pushButton_6->setEnabled(type);
    ui->pushButton_8->setEnabled(type);
    ui->pushButton_11->setEnabled(type);
    ui->pushButton_13->setEnabled(type);
    ui->pushButton_7->setEnabled(type);
    ui->pushButton_9->setEnabled(type);
    if (type == 0) {
        portsUpdate(portInfo, ui->comboBox);
        if (portsAvailable(portInfo)) {
            ui->pushButton->setText("Подключить");
        } else {
            ui->pushButton->setText("Обновить");
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
    if (ui->pushButton->text() == "Подключить") {
        comPort->setPortName(ui->comboBox->currentText());
        if (comPort->open(QIODevice::ReadWrite)) {
            ui->pushButton->setText("Отключить");
            ui->textEdit->append("Connected (System)");
            status(1);
        } else {
            portsUpdate(portInfo, ui->comboBox);
            if (!portsAvailable(portInfo)) ui->pushButton->setText("Обновить");
            ui->textEdit->append("Can't connect (System)");
        }
    } else if (ui->pushButton->text() == "Отключить") {
        comPort->close();
        status(0);
        portsUpdate(portInfo, ui->comboBox);
        if (portsAvailable(portInfo)) {
            ui->pushButton->setText("Подключить");
        } else {
            ui->pushButton->setText("Обновить");
        }
    } else if (ui->pushButton->text() == "Обновить") {
        portsUpdate(portInfo, ui->comboBox);
        if (portsAvailable(portInfo)) ui->pushButton->setText("Подключить");
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    comPort->write("G0 X100 Y100 Z0", 16);
    ui->textEdit->append("G00 X100 Y100 Z00 (user)");
}

void MainWindow::on_pushButton_4_clicked()
{
    short out = serialWrite(comPort, "G00 X00 Y00 Z00", 16);
    if (out == 1) {
        ui->textEdit->append("G00 X00 Y00 Z00 (user)");
        axisX = axisY = axisZ = 0;
    }
    else if (out == -1 || out == -2) {
        ui->textEdit->append("Connection BROKEN (System)");
        status(0);
    }
}

void MainWindow::on_action_exit_triggered()
{
    this->close();
}

void MainWindow::on_pushButton_5_clicked()
{
    ui->textEdit->clear();
}

void MainWindow::on_pushButton_7_pressed()
{
    dy = 5;
    timerY->start(30);
}

void MainWindow::on_pushButton_7_released()
{
    dy = 0;
    timerY->stop();
}

void MainWindow::on_pushButton_13_pressed()
{
    dy = -5;
    timerY->start(30);
}

void MainWindow::on_pushButton_13_released()
{
    dy = 0;
    timerY->stop();
}

void MainWindow::on_pushButton_9_pressed()
{
    dx = 2;
    timerX->start(10);
}

void MainWindow::on_pushButton_9_released()
{
    dx = 0;
    timerX->stop();
}

void MainWindow::on_pushButton_11_pressed()
{
    dx = -2;
    timerX->start(10);
}

void MainWindow::on_pushButton_11_released()
{
    dx = 0;
    timerX->stop();
}
