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

    /* Файл конфигурации */
    config = new Config("settings.conf");

    /* Создаём нужные объекты */
    ioTimer         = new QTimer();
    mainTimer       = new QTimer();
    previewTimer    = new QTimer();
    g_code          = new GCode();
    penColor        = new QColor();
    penColorLight   = new QColor();
    scene           = new QGraphicsScene();
    comPort         = new QSerialPort();
    portInfo        = new QSerialPortInfo();

    /* Определяем последовательный порт */
    comPort->setFlowControl(QSerialPort::NoFlowControl);
    comPort->setDataBits(QSerialPort::Data8);

    /* Читаем конфигурацию */
    settingsRead();

    /* Подключаем сигналы и слоты */
    connect(ioTimer, SIGNAL(timeout()), this, SLOT(io_update_timer()));
    connect(mainTimer, SIGNAL(timeout()), this, SLOT(data_exchange_timer()));
    connect(previewTimer, SIGNAL(timeout()), this, SLOT(preview_update_timer()));

    /* Запускаем таймеры */
    ioTimer->start(IO_TIMER_DELAY);
    previewTimer->start(PREVIEW_TIMER_DELAY);

    ui->graphicsView->setScene(scene);

    /* Выполняем начальный функционал */
    consoleWrite("******** WELCOME TO CNCRUN ********", ui->console);
    uiUpdate();

    //Обновляем цвет пера
    *penColor        = QTextEdit().palette().color(QPalette::WindowText);
    *penColorLight   = QTextEdit().palette().color(QPalette::Highlight);

    /* Указываем иконки */
    this->setWindowIcon(QIcon("cncrun.png"));
    if (penColor->red() + penColor->green() + penColor->blue() > 300) {
        ui->action_open->setIcon(QIcon("icons/openfile_white.png"));
        ui->action_settings->setIcon(QIcon("icons/settings_white.png"));
        ui->action_about->setIcon(QIcon("icons/about_white.png"));
    } else {
        ui->action_open->setIcon(QIcon("icons/openfile_black.png"));
        ui->action_settings->setIcon(QIcon("icons/settings_black.png"));
        ui->action_about->setIcon(QIcon("icons/about_black.png"));
    }
    ui->action_exit->setIcon(QIcon("icons/exit.png"));

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
    int index = box->currentIndex();
    box->clear();
    if (serialAvailable(port)) {
        for (char i = 0; i < port->availablePorts().size(); i++) {
            box->addItem(port->availablePorts()[i].portName());
        }
        if (port->availablePorts().size() > index) box->setCurrentIndex(index);
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

    //Читаем параметры из файла
    settingsRead();

    /* Если это файл с кодом */
    if (path.indexOf(".txt") > 0) {
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            *g_code = QString::fromUtf8(file.readAll().toStdString().c_str());
            ui->gcode_edit->setText(g_code->getString());
        } else {
            QMessageBox box(QMessageBox::Warning, "Невозможно открыть файл", "Не удалось открыть выбранный файл");
            box.exec();
        }
    }
    /* Если это картинка */
    else if (path.indexOf(".png") > 0 || path.indexOf(".jpg") > 0) {
        QImage img(path);
        QString gcode_temp;
        QColor pixelColor;

        /* Создаём динамический массив пикселей */
        bool **pixels = new bool*[img.width()];
        for (int i = 0; i < img.width(); i++) pixels[i] = new bool[img.height()];

        /* Сканируем каждый пиксель картинки */
        if (img.width() != (xsteps + 1) || img.height() != (ysteps + 1)) {

            //Создаём заготовленный текст для сообщения
            QString str = "Выбранное изображение не подходит для преобразования в G-code\nРазмер должен быть строго ";
            str += QString::number(xsteps + 1) + "x" + QString::number(ysteps + 1) + "\n\n";
            str += "Ваше изображение имеет разрешение " + QString::number(img.width()) + "x" + QString::number(img.height());

            //Выводим сообщение
            QMessageBox box(QMessageBox::Warning, "Изображение не подходит", str);
            box.exec();
            return;
        }

        /* Сканируем каждый пиксель картинки */
        for (int x = 0; x < img.width(); x++) { //По X
            for (int y = 0; y < img.height(); y++) { //По Y
                pixelColor = img.pixel(x, y);
                if (pixelColor.red() + pixelColor.red() + pixelColor.red() > 381) {
                    pixels[x][y] = 0; //Если это светлый цвет
                } else {
                    pixels[x][y] = 1; //Если это тёмный цвет
                }
            }
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

        //Если основная ось - X
        if (xisgeneral) {
            y = 0 + (ysteps * changeaxis);
            x = -1;
        }
        //Если основная ось - Y
        else {
            x = 0 + (xsteps * changeaxis);
            y = -1;
        }

        if (!xisgeneral) while((x < img.width() && !changeaxis) || (x >= 0 && changeaxis)) { // *** Пока не дошли до крайних координат ***
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
                    //Если изменился 'x'
                    if (_x != x) {
                        gcode_temp += "G00 X" + QString::number(x) + '\n';
                        _x = x;
                    }

                    gcode_temp += "G00 Y" + QString::number(gap) + '\n';
                    gcode_temp += "G00 Z" + QString::number(zmin) + "\n";
                    if (gap != y) {
                        if (pixels[x][y]) {
                            gcode_temp += "G00 Y" + QString::number(y) + '\n';
                        } else {
                            gcode_temp += "G00 Y" + QString::number(y + 1 - (2 * direction)) + '\n';
                        }
                    }
                    gcode_temp += "G00 Z" + QString::number(zmax) + "\n";
                    gap_now = 0;
                }
            }

            //Если нужно менять направление
            if (dirchange == true) {
                if (direction) y++;
                else y--;
                direction = !direction;
            }
            //Если направление не меняем, то просто обнуляем 'y'
            else {
                y = -1;
            }

            //Сдвигаемся на один столбец
            if (changeaxis)
                x -= step_filling;
            else
                x += step_filling;
        } else while((y < img.height() && !changeaxis) || (y >= 0 && changeaxis)) { // *** Пока не дошли до крайних координат ***
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
                    //Если изменился 'y'
                    if (_y != y) {
                        gcode_temp += "G00 Y" + QString::number(y) + '\n';
                        _y = y;
                    }

                    gcode_temp += "G00 X" + QString::number(gap) + '\n';
                    gcode_temp += "G00 Z" + QString::number(zmin) + "\n";
                    if (gap != x) {
                        if (pixels[x][y]) {
                            gcode_temp += "G00 X" + QString::number(x) + '\n';
                        } else {
                            gcode_temp += "G00 X" + QString::number(x + 1 - (2 * direction)) + '\n';
                        }
                    }
                    gcode_temp += "G00 Z" + QString::number(zmax) + "\n";
                    gap_now = 0;
                }
            }

            //Если нужно менять направление
            if (dirchange == true) {
                if (direction) x++;
                else x--;
                direction = !direction;
            }
            //Если направление не меняем, то просто обнуляем 'x'
            else {
                x = -1;
            }

            //Сдвигаемся на одну строчку
            if (changeaxis)
                y -= step_filling;
            else
                y += step_filling;
        }

        /* Удаляем динамический массив пикселей */
        for (int i = 0; i < img.width(); i++) delete[] pixels[i];
        delete[] pixels;

        /* Выводим полученный код */
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

    if (comPort->isOpen())
        ui->button_com->setText("Отключить");
    else {
        ui->button_com->setText("Подключить");
    }

    io_update_timer();
}

///Рисует картинку предпросмотра
void    MainWindow::previewRender(QString gcode, QGraphicsScene* graphicsscene, int width, int height, bool scaling = 0, int line_exec = 0) {

    /* Адаптивный режим просмотра */
    double scaleX = 1, scaleY = 1;
    if (scaling) {
        scaleX = (height * (tableWidth / tableHeight)) / width;
        scaleY = (width * (tableHeight / tableWidth)) / height;

        if (scaleX > scaleY) scaleY = 1;
        else scaleX = 1;

        width   *= scaleX;
        height  *= scaleY;
    }

    /* Определяем масштабирование */
    double scalex = (graphicsscene->width() - 20)  / width;    //По ширине
    double scaley = (graphicsscene->height() - 20) / height;   //По высоте
    double mainScale;                                   //Итоговый масштаб

    if (scalex > scaley) {
        mainScale = scaley;
    } else {
        mainScale = scalex;
    }

    //Сдвиг картинки по X, чтобы она оказалась в центре сцены
    double dy = ((graphicsscene->height() - 5) - height * mainScale) / 2;
    //Сдвиг картинки по Y, чтобы она оказалась в центре сцены
    double dx = ((graphicsscene->width() - 5) - width * mainScale) / 2;

    /* Настройка пера */
    QPen pen;
    int pen_size;
    if (ui->penSize->value() == 0) {
        pen_size = int(mainScale - 1 + scaleX + scaleY + scaling);            //Если размер пера определяется автоматически
    } else {
        pen_size =  ui->penSize->value();                   //Если размер пера определяется слайдером вручную
    }


    if (pen_size < 1) pen_size = 1;
    pen.setWidth(pen_size);

    graphicsscene->clear();

    /* Читаем каждую команду и отрисовывем картинку с учётом масштабирования */
    QString tmp; tmp.clear();
    int     currentx = 0, currenty = 0, currentz = 0;   //Текущие координаты
    int     value[4];                                   //Значение каждого параметра G-code (G, X, Y, Z)
    bool    exist[4];                                   //Был ли указан параметр (G, X, Y, Z)
    short   index = 0;                                  //К какому параметру относится значение (при чтении строки)
    int     z_min = 1000;
    int     z_max = 0;
    int     str_num = 0;

    for (int i = 0; i < gcode.length(); i++) {
        //Берём одну конкретную строчку
        if (gcode[i] == '\n') {
            exist[0] = exist[1] = exist[2] = exist[3] = 0;
            //Читаем строчку
            for (int a = 0; a < tmp.length(); a++) {
                if (tmp[a] == 'G') {
                    exist[0] = 1;
                    value[0] = 0;
                    index = 0;
                } else if (tmp[a] == 'X') {
                    exist[1] = 1;
                    value[1] = 0;
                    index = 1;
                } else if (tmp[a] == 'Y') {
                    exist[2] = 1;
                    value[2] = 0;
                    index = 2;
                } else if (tmp[a] == 'Z') {
                    exist[3] = 1;
                    value[3] = 0;
                    index = 3;
                } else if (tmp[a] > 47 && tmp[a] < 58) {
                    value[index] = (value[index] * 10) + (tmp.toUtf8().at(a) - 48);
                }
            }

            //Отрисовываем линию
            if (exist[0]) {
                //Меняем цвет
                if (str_num > line_exec)
                    pen.setColor(*penColor);
                else
                    pen.setColor(*penColorLight);

                //Если была указана ось Z
                if (exist[3]) {
                    currentz = value[3];
                    if (currentz < z_min && z_max > 0) z_min = currentz;
                    if (currentz > z_max) z_max = currentz;
                }
                //Если была указана ось X
                if (exist[1]) {
                    if (currentz == z_min) {
                        graphicsscene->addLine(currentx * mainScale * scaleX + dx, currenty * mainScale * scaleY + dy, value[1] * mainScale * scaleX + dx, currenty * mainScale * scaleY + dy, pen);
                    }
                    currentx = value[1];
                }

                //Если была указана ось Y
                if (exist[2]) {
                    if (currentz == z_min) {
                        graphicsscene->addLine(currentx * mainScale * scaleX + dx, currenty * mainScale * scaleY + dy, currentx * mainScale * scaleX + dx, value[2] * mainScale * scaleY + dy, pen);
                    }
                    currenty = value[2];
                }
            }

            str_num++;
            tmp.clear();
        } else tmp += gcode.at(i);
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

void MainWindow::preview_update_timer() {
    if (projectWorking) return; //Если проект запущен, то картинка и так перерисовывается автоматически

    //Создаём переменные для запоминания предыдущих размеров сцены
    static int sceneLastWidth = 0;
    static int sceneLastHeight = 0;

    //Если изменились размеры сцены
    if (sceneLastWidth != ui->graphicsView->width() || sceneLastHeight != ui->graphicsView->height()) {
        //Обновляем размеры графической сцены
        scene->setSceneRect(0, 0, ui->graphicsView->width(), ui->graphicsView->height());

        //Обновляем цвет пера
        *penColor        = QTextEdit().palette().color(QPalette::WindowText);
        *penColorLight   = QTextEdit().palette().color(QPalette::Highlight);

        //Копируем текст G-code
        QString str = ui->gcode_edit->toPlainText();

        //Показываем картинку
        previewRender(str, scene, xsteps + 1, ysteps + 1, previewscaling);

        //Запоминаем размеры
        sceneLastWidth  = ui->graphicsView->width();
        sceneLastHeight = ui->graphicsView->height();
    }

}

void MainWindow::io_update_timer() {
    if (projectWorking) return;

    static int ports = 0;

    serialUpdate(portInfo, ui->comboBox);

    if (ports != serialAvailable(portInfo)) {
        ports = serialAvailable(portInfo);

        /* Выводим названия найденных портов */
        QString str;
        bool    portExist = 0;

        if (ports < 2) str = "Found device: ";
        else str = "Found devices: ";

        for (char i = 0; i < portInfo->availablePorts().size(); i++) {
            str += portInfo->availablePorts()[i].portName() + ' ';
            //Если подключенный ранее порт встретился
            if (portInfo->availablePorts()[i].portName() == comPort->portName()) portExist = 1;
        }

        str += " [SYSTEM]";

        //Если мы подключены, но порта такого уже нет
        if (comPort->isOpen() && !portExist) {
            consoleWrite("Connection LOST [SYSTEM]", ui->console);
            comPort->close();
        }

        if (ports > 0) consoleWrite(str, ui->console);
    }

    if (comPort->isOpen()) {
        ui->button_com->setText("Отключить");
        ui->comboBox->setEnabled(0);
    } else {
        settingsRead();
        ui->button_com->setText("Подключить");
        ui->comboBox->setEnabled(1);
    }

    /* Выводим информацию о текущей настройке порта */
    ui->labelBaudRate->setText(QString::number(comPort->baudRate()));
    if (comPort->parity() == QSerialPort::NoParity)
        ui->labelParity->setText("Нет");
    else
        ui->labelParity->setText("Да");
    ui->labelStopBits->setText(QString::number(comPort->stopBits()));
    ui->labelTimeout->setText(QString::number(timeout));


    ui->button_com->setEnabled(ports);
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
        consoleWrite("Connection LOST [SYSTEM]", ui->console);
        projectWorking = 0;
        return;
    } else {
        data = comPort->readAll();
        consoleWrite("Sent: " + g_code->getCommand(i), ui->console);
        time = timeout*1000/MAIN_TIMER_DELAY;
    }

    /* Показываем статистику */
    ui->progressBar->setValue((i + 1) * 100 / g_code->size());

    //Обновляем цвет пера
    *penColor        = QTextEdit().palette().color(QPalette::WindowText);
    *penColorLight   = QTextEdit().palette().color(QPalette::Highlight);

    //Обновляем размеры графической сцены
    scene->setSceneRect(0, 0, ui->graphicsView->width(), ui->graphicsView->height());

    //Показываем картинку
    previewRender(g_code->getString(), scene, xsteps + 1, ysteps + 1, previewscaling, i);

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

        mainTimer->start(MAIN_TIMER_DELAY);
    } else {
        projectWorking = 0;
    }
    uiUpdate();
}

void MainWindow::on_button_com_clicked()
{
    if (!comPort->isOpen()) {
        /* Читаем конфигурацию */
        settingsRead();

        //Указываем имя порта
        comPort->setPortName(ui->comboBox->currentText());

        if (comPort->open(QIODevice::ReadWrite)) {
            //Если удалось подключиться
            consoleWrite("Connected to " + ui->comboBox->currentText() + " [SYSTEM]", ui->console);
        } else {
            //Если подключение не удалось
            consoleWrite("Cannot connect to " + ui->comboBox->currentText() + " [SYSTEM]", ui->console);
        }
    } else {
        comPort->close();
    }

    //Обновляем интерфейс
    uiUpdate();
}

void MainWindow::on_action_open_triggered()
{
    //Если уже идёт выполнение
    if (projectWorking) {
        consoleWrite("", ui->console);
        consoleWrite("Project is already running, aborted [SYSTEM]", ui->console);
        consoleWrite("", ui->console);
        return;
    }

    QFileDialog fileDialog;
    QString str = fileDialog.getOpenFileName(this, tr("Открыть"), "", tr("Изображение или G-code (*.jpg *.png *.txt)"));
    if (str.size() != 0) fileOpen(str);
}

void MainWindow::on_button_home_clicked()
{
    QString str = "G00 X00 Y00 Z" + QString::number(zmax);
    if (serialWrite(comPort, str.toStdString().c_str(), 17) < 0) {
        uiUpdate();
        consoleWrite("Connection LOST [SYSTEM]", ui->console);
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
    on_button_send_clicked();
}

void MainWindow::on_button_send_clicked()
{
    if (ui->console_line->text().isEmpty()) return;
    if (serialWrite(comPort, ui->console_line->text().toStdString().c_str(), ui->console_line->text().length() + 1) < 0) {
        uiUpdate();
        consoleWrite("Connection LOST [SYSTEM]", ui->console);
    } else consoleWrite("Sent: " + ui->console_line->text() + " [USER]", ui->console);
}

void MainWindow::on_action_settings_triggered()
{
    Settings settings;
    settings.setModal(true);
    settings.exec();
}

void MainWindow::on_gcode_edit_textChanged()
{
    //Копируем текст с G-code
    QString str = ui->gcode_edit->toPlainText();

    //Обновляем размеры графической сцены
    scene->setSceneRect(0, 0, ui->graphicsView->width(), ui->graphicsView->height());

    //Подсчитываем количество строк
    int numofcom = 0;
    for (int i = 0; i < str.length(); i++) if (str.at(i) == '\n') numofcom++;
    ui->label_8->setText(QString::number(numofcom));

    //Показываем картинку
    previewRender(str, scene, xsteps + 1, ysteps + 1, previewscaling);
}

void MainWindow::on_checkBox_stateChanged(int arg)
{
    previewscaling = arg;
    if (!projectWorking) on_gcode_edit_textChanged();
}

void MainWindow::settingsRead() {
    /* Проверяем, есть ли файл конфигурации */
    if (!config->isexist()) {

        //Пишем и оповещаем о том, что отсутствует файл конфигурации
        consoleWrite("No configuration file [SYSTEM]", ui->console);
        config->make("");
        consoleWrite("Configuration file created automatically [SYSTEM]", ui->console);
        QMessageBox box(QMessageBox::Information, "Не найден файл конфигурации", "Не удалось обнаружить файл конфигурации!\nАвтоматически создан пустой файл конфигурации.\nПожалуйста, укажите параметры вашего оборудования");
        box.exec();

        //Открываем настройки
        Settings settings;
        settings.setModal(true);
        settings.exec();
    }
    config->read();

    /* Указываем новые значения по файлу конфигурации */
    for (int i = 0; i < config->count(); i++) {
        if (config->parameter(i) == "step_filling") step_filling = config->value(i).toInt();
        else if (config->parameter(i) == "main_axis") {
            if (config->value(i) == "x") xisgeneral = 1;
            else xisgeneral = 0;
        }
        else if (config->parameter(i) == "axisx_max") xsteps = config->value(i).toInt();
        else if (config->parameter(i) == "axisy_max") ysteps = config->value(i).toInt();
        else if (config->parameter(i) == "axisz_down") zmin = config->value(i).toInt();
        else if (config->parameter(i) == "axisz_up") zmax = config->value(i).toInt();
        else if (config->parameter(i) == "changeaxis") {
            if (config->value(i) == "true" || config->value(i) == "1") changeaxis = 1;
            else changeaxis = 0;
        }
        else if (config->parameter(i) == "dirchange") {
            if (config->value(i) == "true" || config->value(i) == "1") dirchange = 1;
            else dirchange = 0;
        }
        else if (config->parameter(i) == "baud_rate") comPort->setBaudRate(config->value(i).toInt());
        else if (config->parameter(i) == "parity") {
            if (config->value(i) == "no") comPort->setParity(QSerialPort::NoParity);
            else comPort->setParity(QSerialPort::EvenParity);
        }
        else if (config->parameter(i) == "stop_bits") {
            if (config->value(i) == "1") comPort->setStopBits(QSerialPort::OneStop);
            else comPort->setStopBits(QSerialPort::TwoStop);
        }
        else if (config->parameter(i) == "timeout") timeout = config->value(i).toInt();
        else if (config->parameter(i) == "answer") answer = config->value(i);
        else if (config->parameter(i) == "tablewidth") tableWidth = config->value(i).toInt();
        else if (config->parameter(i) == "tableheight") tableHeight = config->value(i).toInt();
    }
}

void MainWindow::on_penSize_valueChanged(int value)
{
    if (value == 0) {
        ui->penValue->setText("автоматический");
    } else {
        ui->penValue->setText(QString::number(value));
    }

    //Обновляем размеры графической сцены
    scene->setSceneRect(0, 0, ui->graphicsView->width(), ui->graphicsView->height());

    //Обновляем цвет пера
    *penColor        = QTextEdit().palette().color(QPalette::WindowText);
    *penColorLight   = QTextEdit().palette().color(QPalette::Highlight);

    //Копируем текст G-code
    QString str = ui->gcode_edit->toPlainText();

    //Показываем картинку
    previewRender(str, scene, xsteps + 1, ysteps + 1, previewscaling);
}
