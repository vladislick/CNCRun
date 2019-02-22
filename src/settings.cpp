#include "lib/settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
    file = new ConfFile("settings.conf");
    if (!file->read()) {
        QMessageBox::critical(this, "Ошибка", "Отсутствует файл конфигурации\n Будет создан новый файл");
        if (!file->create()) {
            QMessageBox::critical(this, "Ошибка", "Не получилось создать новый файл конфигурации!");
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
