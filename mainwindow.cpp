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

    consoleWrite("Welcome!", ui->console);

    conffile = new ConfFile("settings.conf");

    /* Определяем последовательный порт */
    comPort = new QSerialPort();
    portInfo = new QSerialPortInfo();
    comPort->setBaudRate(QSerialPort::Baud9600);
    comPort->setFlowControl(QSerialPort::NoFlowControl);
    comPort->setParity(QSerialPort::NoParity);
    comPort->setDataBits(QSerialPort::Data8);
    comPort->setStopBits(QSerialPort::OneStop);

    /* Подключаем сигналы и слоты */
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

///Возвращает количество доступных портов
int MainWindow::serialAvailable(QSerialPortInfo* port) {
    return port->availablePorts().size();
}

///Обрабатывает открытие файла
void MainWindow::fileOpen(QString path) {
    QFile file(path);
    if (path.indexOf(".txt")) {
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            gcode = QString::fromUtf8(file.readAll().toStdString().c_str());
            ui->gcode_edit->setText(gcode);
        } else {
            QMessageBox box(QMessageBox::Critical, "Невозможно открыть файл", "Не удалось открыть выбранный файл");
            box.exec();
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

void MainWindow::on_button_start_clicked()
{
    gcode = ui->gcode_edit->toPlainText();
}

void MainWindow::on_button_com_clicked()
{

}

void MainWindow::on_action_open_triggered()
{
    QFileDialog fileDialog;
    QString str = fileDialog.getOpenFileName(this, tr("Открыть"), "", tr("Изображение или G-code (*.jpg *.png *.txt)"));
    if (str.size() != 0) fileOpen(str);
}

///Старт работы станка
void MainWindow::start() {

}
