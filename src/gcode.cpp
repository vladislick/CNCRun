#include "lib/gcode.h"

GCode::GCode()
{

}

///Генерация G-code из строки
GCode:: GCode(QString str) {
    gcode_str = str;
}

///Общее количество команд
int GCode::size() {
    return gcode_list.size();
}

///Сгенерировать G-code из строки (возвращает ошибки, если есть)
int GCode::generate() {
    gcode_size = 0;
    gcode_list.clear();
    QString str_temp;
    bool isgcode = 0;

    /* Проходим по каждому символу строки */
    for (int i = 0; i < gcode_str.length(); i++) {
        if (gcode_str[i] == 'G') {
            isgcode = 1;
            str_temp.clear();
        }
        if (gcode_str[i] == '\n' || i == (gcode_str.length() - 1)) {
            isgcode = 0;
            if (i == (gcode_str.length() - 1)) str_temp += gcode_str[i];
            /* Удаляем лишние пробелы */
            while(str_temp.indexOf(' ') == 0) {
                str_temp.remove(0, 1);
            }
            while(str_temp.lastIndexOf(' ') == (str_temp.length() - 1)) {
                str_temp.remove((str_temp.length() - 1), 1);
            }

            /* Добавляем команду */
            gcode_list += str_temp;
        }
        if (isgcode) {
            str_temp += gcode_str[i];
        }
    }

    return 0;
}

///Получить команду
QString GCode::getCommand(int index) {
    return gcode_list[index];
}

///Получить исходную строку
QString GCode::getString() {
    return gcode_str;
}

///Перегрузка оператора присваивания
GCode GCode::operator=(QString str) {
    gcode_str = str;
    return *this;
}

///Перегрузка оператора выборки элемента
QString GCode::operator[](int index) {
    return gcode_list[index];
}
