#include "lib/mainwindow.h"
#include "lib/aboutapp.h"
#include "lib/settings.h"
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

    /* Определяем последовательный порт */
    comPort = new QSerialPort();
    portInfo = new QSerialPortInfo();
    comPort->setBaudRate(QSerialPort::Baud9600);
    comPort->setFlowControl(QSerialPort::NoFlowControl);
    comPort->setParity(QSerialPort::NoParity);
    comPort->setDataBits(QSerialPort::Data8);
    comPort->setStopBits(QSerialPort::OneStop);

    mainTimer = new QTimer();
    g_code = new GCode();

    /* Подключаем сигналы и слоты */
    connect(mainTimer, SIGNAL(timeout()), this, SLOT(data_exchange_timer()));

    /* Выполняем начальный функционал */
    consoleWrite("<<<<< WELCOME TO CNCRUN >>>>>", ui->console);
    uiUpdate();

    /* Разбиремся с файлом конфигурации */
    config = new Config("settings.conf");

    /* Указываем базовые значения переменных конфигурации */
    step_filling    = 1;
    xisgeneral      = 1;
    xsteps          = 240;
    ysteps          = 160;
    zmin            = 30;
    zmax            = 40;
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
            *g_code = QString::fromUtf8(file.readAll().toStdString().c_str());
            ui->gcode_edit->setText(g_code->getString());
        } else {
            QMessageBox box(QMessageBox::Critical, "Невозможно открыть файл", "Не удалось открыть выбранный файл");
            box.exec();
        }
    }
    /* Если это картинка */
    else if (path.indexOf(".png") > 0 || path.indexOf(".jpg") > 0) {
        QImage img(path);
        QString gcode_temp;
        QColor pixelColor;
        bool pixels[240][160];

        /* Сканируем каждый пиксель картинки */
        if (img.width() == xsteps && img.height() == ysteps) {
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
        } else {
            QMessageBox box(QMessageBox::Critical, "Изображение не подходит", "Выбранное изображение не подходит для преобразования в G-code (размер должен быть строго 240x160)");
            box.exec();
            return;
        }

        /* Проверяем, есть ли файл конфигурации */
        if (!config->isexist()) {
            config->make("");
            consoleWrite("NEW CONFIG FILE WAS CREATED", ui->console);
        }
        config->read();

        /* Указываем новые значения по файлу конфигурации */
        for (int i = 0; i < config->count(); i++) {
            if (config->parameter(i) == "step_filling") step_filling = config->value(i).toInt();
            else if (config->parameter(i) == "main_axis") {
                if (config->value(i) == "x") xisgeneral = 1;
                else xisgeneral = 0;
            } else if (config->parameter(i) == "axisx_max") xsteps = config->value(i).toInt();
            else if (config->parameter(i) == "axisy_max") ysteps = config->value(i).toInt();
            else if (config->parameter(i) == "axisz_down") zmin = config->value(i).toInt();
            else if (config->parameter(i) == "axisz_up") zmax = config->value(i).toInt();
        }

        /* Генерируем G-code */
        gcode_temp.clear();
        gcode_temp += "G00 Z" + QString::number(zmax) + "\nG00 X00 Y00\n"; //Встаём на нулевую координату

        bool direction = 1;
        bool gap_now = 0;
        int  gap = 0;
        int  x;
        int  y;
        int  _x = -1;
        int  _y = -1;

        if (xisgeneral) {
            x = -1;
            y = 0;
        } else {
            x = 0;
            y = -1;
        }

        if (!xisgeneral) while(x < img.width()) {
            while (!(y == (img.height() - 1) && direction) && !(y == 0 && !direction)) {
                //Если вниз
                if (direction) y++;
                //Если вверх
                else y--;

                /* Делим все пиксели в столбце на промежутки и добавляем в G-code */
                if (pixels[x][y]) {
                    if (!gap_now) {
                        gap = y;
                        gap_now = 1;
                    }
                }
                if ((gap_now && !pixels[x][y]) || (y == (img.height() - 1) && direction && pixels[x][y]) || (y == 0 && !direction && pixels[x][y])) {
                    if (_x != x) {
                        gcode_temp += "G00 X" + QString::number(x) + '\n';
                        _x = x;
                    }
                    gcode_temp += "G00 Y" + QString::number(gap) + '\n';
                    gcode_temp += "G00 Z" + QString::number(zmin) + "\n";
                    if (gap != y) gcode_temp += "G00 Y" + QString::number(y) + '\n';
                    gcode_temp += "G00 Z" + QString::number(zmax) + "\n";
                    gap_now = 0;
                }
            }
            if (direction) y++;
            else y--;
            direction = !direction;
            x += step_filling;
        } else while(y < img.height()) {
            while (!(x == (img.width() - 1) && direction) && !(x == 0 && !direction)) {
                //Если вправо
                if (direction) x++;
                //Если влево
                else x--;

                /* Делим все пиксели в столбце на промежутки и добавляем в G-code */
                if (pixels[x][y]) {
                    if (!gap_now) {
                        gap = x;
                        gap_now = 1;
                    }
                }
                if ((gap_now && !pixels[x][y]) || (x == (img.width() - 1) && direction && pixels[x][y]) || (x == 0 && !direction && pixels[x][y])) {
                    if (_y != y) {
                        gcode_temp += "G00 Y" + QString::number(y) + '\n';
                        _y = y;
                    }
                    gcode_temp += "G00 X" + QString::number(gap) + '\n';
                    gcode_temp += "G00 Z" + QString::number(zmin) + "\n";
                    if (gap != x) gcode_temp += "G00 X" + QString::number(x) + '\n';
                    gcode_temp += "G00 Z" + QString::number(zmax) + "\n";
                    gap_now = 0;
                }
            }
            if (direction) x++;
            else x--;
            direction = !direction;
            y += step_filling;
        }

        *g_code = gcode_temp;
        ui->gcode_edit->setText(gcode_temp);
    }
}

///Обновляет интерфейс программы
void MainWindow::uiUpdate() {
    bool isActive = comPort->isOpen();
    if (projectWorking) isActive = 0;

    ui->button_start->setEnabled(comPort->isOpen());
    ui->button_home->setEnabled(isActive);
    ui->console_line->setEnabled(isActive);
    ui->button_send->setEnabled(isActive);
    ui->gcode_edit->setReadOnly(projectWorking);
    ui->progressBar->setEnabled(projectWorking);

    if (projectWorking) {
        ui->button_start->setText("Остановить");
    } else {
        ui->button_start->setText("Начать");
        ui->progressBar->setValue(0);
    }

    if (comPort->isOpen()) ui->button_com->setText("Отключить");
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
    static int i = 0;
    static int time = 0;
    QByteArray data;

    /* Если работа завершена аварийно */
    if (!projectWorking) {
        consoleWrite("\n[PROJECT STOPPED]\n", ui->console);
        time = i = 0;
        mainTimer->stop();
        uiUpdate();
        return;
    }

    /* Если ждём ответа от станка */
    if (time > 0) {
        data = comPort->readAll();
        //Если ответ пришёл
        if (data == "y") {
            time = 0;
            return;
        }
        time--;
        if (time == 0) time = -1;
        return;
    }
    /* Если не дождались ответа */
    else if (time < 0) {
        consoleWrite("No answer from CNC, Sorry [SYSTEM]", ui->console);
        projectWorking = 0;
        return;
    }

    /* Отправляем команду */
    if (serialWrite(comPort, g_code->getCommand(i).toStdString().c_str(), g_code->getCommand(i).length() + 1) < 0) {
        consoleWrite("Connection BROKEN, Sorry [SYSTEM]", ui->console);
        projectWorking = 0;
        return;
    } else {
        data = comPort->readAll();
        consoleWrite("Sent: " + g_code->getCommand(i), ui->console);
        time = 5000;
    }

    /* Показываем статистику */
    ui->progressBar->setValue((i + 1) * 100 / g_code->size());

    /* Если все команды были успешно завершены */
    if (i < (g_code->size() - 1)) i++;
    else {
        consoleWrite("\n[PROJECT COMPLETED]\n", ui->console);
        projectWorking = 0;
        mainTimer->stop();
        time = i = 0;
        uiUpdate();
    }
}

void MainWindow::on_button_start_clicked()
{
    if (ui->button_start->text() == "Начать") {
        *g_code = ui->gcode_edit->toPlainText();
        g_code->generate();

        if (g_code->size() == 0) {
            consoleWrite("G-code field is empty! [SYSTEM]", ui->console);
            return;
        }

        projectWorking = 1;

        consoleWrite("\n[PROJECT STARTED]\n", ui->console);

        mainTimer->start(2);
    } else {
        projectWorking = 0;
    }
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

void MainWindow::on_console_line_returnPressed()
{
    if (ui->console_line->text().isEmpty()) return;
    if (serialWrite(comPort, ui->console_line->text().toStdString().c_str(), ui->console_line->text().length() + 1) < 0) {
        uiUpdate();
        consoleWrite("Connection BROKEN, Sorry [SYSTEM]", ui->console);
    } else consoleWrite("Sent: " + ui->console_line->text() + " [USER]", ui->console);
}

void MainWindow::on_button_send_clicked()
{
    if (ui->console_line->text().isEmpty()) return;
    if (serialWrite(comPort, ui->console_line->text().toStdString().c_str(), ui->console_line->text().length() + 1) < 0) {
        uiUpdate();
        consoleWrite("Connection BROKEN, Sorry [SYSTEM]", ui->console);
    } else consoleWrite("Sent: " + ui->console_line->text() + " [USER]", ui->console);
}

void MainWindow::on_action_settings_triggered()
{
    Settings settings;
    settings.setModal(true);
    settings.exec();
}
