#include "lib/settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);

    /* Указываем иконку */
    this->setWindowIcon(QIcon("cncrun.png"));

    /* Разбиремся с файлом конфигурации */
    config = new Config("settings.conf");
    if (!config->isexist()) {
        config->make(" ");
    }
    config->read();

    /* Указываем новые значения по файлу конфигурации */
    for (int i = 0; i < config->count(); i++) {
        if (config->parameter(i) == "axisx_max") {
            ui->spinBox->setValue(config->value(i).toInt());
        } else if (config->parameter(i) == "axisy_max") {
            ui->spinBox_2->setValue(config->value(i).toInt());
        } else if (config->parameter(i) == "axisz_down") {
            ui->spinBox_3->setValue(config->value(i).toInt());
        } else if (config->parameter(i) == "axisz_up") {
            ui->spinBox_4->setValue(config->value(i).toInt());
        } else if (config->parameter(i) == "main_axis") {
            if (config->value(i) == "x") ui->comboBox_4->setCurrentIndex(1);
            else ui->comboBox_4->setCurrentIndex(0);
        } else if (config->parameter(i) == "step_filling") {
            if (config->value(i) == "1") ui->comboBox_6->setCurrentIndex(0);
            else if (config->value(i) == "2") ui->comboBox_6->setCurrentIndex(1);
            else if (config->value(i) == "3") ui->comboBox_6->setCurrentIndex(2);
            else if (config->value(i) == "4") ui->comboBox_6->setCurrentIndex(3);
        } else if (config->parameter(i) == "baud_rate") {
            if (config->value(i) == "9600") ui->comboBox->setCurrentIndex(0);
            else if (config->value(i) == "4800") ui->comboBox->setCurrentIndex(1);
            else if (config->value(i) == "2400") ui->comboBox->setCurrentIndex(2);
            else if (config->value(i) == "1200") ui->comboBox->setCurrentIndex(3);
        } else if (config->parameter(i) == "parity") {
            if (config->value(i) == "no") ui->comboBox_2->setCurrentIndex(0);
            else  ui->comboBox_2->setCurrentIndex(1);
        } else if (config->parameter(i) == "stop_bits") {
            if (config->value(i) == "1") ui->comboBox_3->setCurrentIndex(0);
            else ui->comboBox_3->setCurrentIndex(1);
        } else if (config->parameter(i) == "changeaxis") {
            if (config->value(i) == "true" || config->value(i) == "1") ui->checkBox->setChecked(1);
            else ui->checkBox->setChecked(0);
        } else if (config->parameter(i) == "dirchange") {
            if (config->value(i) == "true" || config->value(i) == "1") ui->checkBox_2->setChecked(1);
            else ui->checkBox_2->setChecked(0);
        } else if (config->parameter(i) == "timeout") {
            ui->spinBoxTimeout->setValue(config->value(i).toInt());
        } else if (config->parameter(i) == "answer") {
            ui->lineEdit->setText(config->value(i));
        } else if (config->parameter(i) == "tablewidth") {
            ui->spinBoxTableWidth->setValue(config->value(i).toInt());
        } else if (config->parameter(i) == "tableheight") {
            ui->spinBoxTableHeight->setValue(config->value(i).toInt());
        }
    }
}

Settings::~Settings()
{
    delete ui;
}

void Settings::on_pushButton_2_clicked()
{
    this->close();
}

void Settings::on_pushButton_clicked()
{
    QString str;

    str = "# ФАЙЛ КОНФИГУРАЦИИ #\n\n";
    str += "[Serial port configuration]\n";

    //Указываем скорость соединения
    str += "baud_rate       = " + ui->comboBox->currentText() + "  //Скорость\n";

    //Указываем проверку чётности
    str += "parity          = ";
    if (ui->comboBox_2->currentIndex() == 0) {
        str += "no ";
    } else {
        str += "yes";
    }
    str += "   //Проверка чётности\n";

    //Указываем стоп-биты
    str += "stop_bits       = " + ui->comboBox_3->currentText() + "     //Стоп-биты\n" ;

    //Указываем время ответа
    str += "timeout         = " + ui->spinBoxTimeout->text() + "    //Время для ответа\n";

    //Указываем ответ от станка
    str += "answer          = " + ui->lineEdit->text() + "     //Ответ от станка\n\n";


    str += "[GCODE configuration]\n";

    //Указываем заполнение
    str += "step_filling    = " + ui->comboBox_6->currentText() + "     //Каждый какой столбец брать\n";

    //Указываем относительно какой оси делать G-code
    str += "main_axis       = ";
    if (ui->comboBox_4->currentIndex() == 0) {
        str += "y";
    } else {
        str += "x";
    }
    str += "     //Относительно какой оси делать G-code\n";

    //Указываем нужно ли изменить направление сканирования
    str += "changeaxis      = ";
    if (ui->checkBox->isChecked()) {
        str += "true  //Изменить направление сканирования\n";
    } else {
        str += "false //Изменить направление сканирования\n";
    }

    //Указываем нужно ли менять направление сканирования
    str += "dirchange       = ";
    if (ui->checkBox_2->isChecked()) {
        str += "true  //Менять направление сканирования\n\n";
    } else {
        str += "false //Менять направление сканирования\n\n";
    }

    str += "[CNC configuration]\n";

    //Указываем максимальное количество шагов по X
    str += "axisx_max   = " + QString::number(ui->spinBox->value()) + "       //Максимальное количество шагов по X\n";

    //Указываем максимальное количество шагов по Y
    str += "axisy_max   = " + QString::number(ui->spinBox_2->value()) + "       //Максимальное количество шагов по Y\n";

    //Указываем значение Z для опущенного инструмента
    str += "axisz_down  = " + QString::number(ui->spinBox_3->value()) + "        //Значение Z для опущенного инструмента\n";

    //Указываем значение Z для поднятого инструмента
    str += "axisz_up    = " + QString::number(ui->spinBox_4->value()) + "        //Значение Z для поднятого инструмента\n";

    //Указываем ширину стола
    str += "tablewidth  = " + QString::number(ui->spinBoxTableWidth->value()) + "        //Физическая ширина стола\n";

    //Указываем длину стола
    str += "tableheight = " + QString::number(ui->spinBoxTableHeight->value()) + "        //Физическая длина стола\n";

    //Пересоздаём файл конфигурации
    config->make(str);
    config->read();

    //Закрываем окно
    this->close();
}

void Settings::on_spinBoxTimeout_valueChanged(int arg)
{
    if (arg > 10 && arg <= 20) {
        ui->labelSeconds->setText("секунд");
        return;
    }

    if ((arg % 10) == 1) ui->labelSeconds->setText("секунда");
    else if ((arg % 10) > 1 && (arg % 10) < 5) ui->labelSeconds->setText("секунды");
    else ui->labelSeconds->setText("секунд");
}
