#include "lib/config.h"

Config::Config(const QString& path)
{
    file = new QFile(path);
    data = new QList<QString>;
    read(); //Читаем данные из файла
}

//Прочитать название параметра
QString Config::parameter(int index) {
    if (index < 0 || (index * 2 + 1) > data->count()) return "";
    return data->at(index * 2);
}

//Прочитать значение параметра
QString Config::value(int index) {
    if (index < 0 || (index * 2 + 2) > data->count()) return "";
    return data->at(index * 2 + 1);
}

//Прочитать файл
bool Config::read() {
    /* Открываем файл, читаем содержимое и закрываем его */
    if (!file->open(QFile::ReadOnly | QFile::Text)) return false;
    QByteArray array = file->readAll();
    file->close();

    /* Сканируем содержимое файла */
    data->clear();
    QString str;
    QString str_last;
    bool gap = 0;
    bool sign = 0;

    for (int i = 0; i < array.count(); i++) {
        if (array[i] == ' ' || array[i] == '\n' || array[i] == '\0' || (i + 1) == array.count()) {
            if (gap) {
                if (sign) *data << str;
                sign = 0;
                str_last = str;
                str.clear();
                gap = 0;
            }
        } else if (array[i] == '=') {
            if (str.isEmpty())
                *data << str_last;
            else
                *data << str;
            str.clear();
            sign = 1;
            gap = 0;
        } else gap = 1;
        if (gap) str += array[i];
    }
    return true;
}

//Создать файл по экземпляру
void Config::make(QString string) {
    file->remove();
    file->open(QFile::WriteOnly);
    QByteArray array;
    array.append(string);
    file->write(array);
    file->close();
}

//Существует ли файл настроек
bool Config::isexist() {
    return file->exists();
}

//Возвращает количество элементов
int Config::count() {
    return data->count() >> 1;
}
