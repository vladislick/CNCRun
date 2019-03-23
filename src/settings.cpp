#include "lib/settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);

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
    str += "stop_bits       = " + ui->comboBox_3->currentText() + "     //Стоп-биты\n\n" ;

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
    str += "     //Относительно какой оси делать G-code\n\n";

    str += "[CNC configuration]\n";

    //Указываем максимальное количество шагов по X
    str += "axisx_max   = " + QString::number(ui->spinBox->value()) + "       //Максимальное количество шагов по X\n";

    //Указываем максимальное количество шагов по Y
    str += "axisy_max   = " + QString::number(ui->spinBox_2->value()) + "       //Максимальное количество шагов по Y\n";

    //Указываем значение Z для опущенного инструмента
    str += "axisz_down  = " + QString::number(ui->spinBox_3->value()) + "        //Значение Z для опущенного инструмента\n";

    //Указываем значение Z для поднятого инструмента
    str += "axisz_up    = " + QString::number(ui->spinBox_4->value()) + "        //Значение Z для поднятого инструмента\n";

    //Пересоздаём файл конфигурации
    config->make(str);
    config->read();

    //Закрываем окно
    this->close();
}
