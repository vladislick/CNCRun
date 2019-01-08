#include "conffile.h"

ConfFile::ConfFile(const QString& filePath)
{
    list = new QList<QString>;
    file = new QFile(filePath);
}

bool ConfFile::read() {
    if (!file->open(QFile::ReadOnly | QFile::Text)) return 0;
    QByteArray array = file->readAll();
    QString str;
    list->clear();
    bool skip = 0;
    for (int i = 0; i < array.size(); i++) {
        //Отсеиваем комментарии и заголовки в файле
        if (array[i] == '\n') {
            if (str.size()) {
                *list << str;
                str.clear();
            }
            skip = 0;
            continue;
        }
        if (skip) continue;
        if (array[i] == '#' || array[i] == ';' || array[i] == '[') {
            skip = 1;
            continue;
        }

        //Считываем параметры
        if (array[i] == ' ' || array[i] == '=') {
            if (str.isEmpty()) continue;
            else {
                *list << str;
                str.clear();
                continue;
            }
        }
        str += array[i];
    }
    if (!first) delete [] strings;
    first = 0;
    strings = new QString[list->count()];
    for (int i = 0; i < list->size(); i++) {
        strings[i] = list->at(i);
    }

    file->close();
    return 1;
}

int ConfFile::parametersNumber() {
    return list->count();
}

QString ConfFile::parameter(int index) {
    return strings[index];
}

bool ConfFile::create() {
    if (!file->open(QFile::ReadOnly | QFile::Text)) {
        //Если файла конфигурации нет, то создаём новый
        file->open(QFile::WriteOnly);
        QByteArray array;
        array.append("# Файл конфигурации\n\n");
        array.append("[Serial port configuration]\n");
        array.append("baud_rate = 9600\n");
        array.append("flow_control = no\n");
        array.append("parity = no\n");
        array.append("stop_bits = 1\n\n");
        array.append("[CNC configuration]\n");
        array.append("axisx_step = 0.00\n");
        array.append("axisy_step = 0.00\n");
        array.append("axisx_max = 240\n");
        array.append("axisy_max = 160\n");
        file->write(array);
        file->close();
        return 1;
    }
    file->close();
    return 0;
}
