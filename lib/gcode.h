#ifndef GCODE_H
#define GCODE_H

#include <QString>
#include <QList>

///Удобная работа с G-code
class GCode
{
public:
    GCode();
    ///Генерация G-code из строки
    GCode(QString str);
    ///Общее количество команд
    int size();
    ///Сгенерировать G-code из строки (возвращает ошибки, если есть)
    int generate();
    ///Получить исходную строку
    QString getString();
    ///Получить команду
    QString getCommand(int index);
    ///Перегрузка оператора присваивания
    GCode operator=(QString str);
    ///Перегрузка оператора выборки элемента
    QString operator[](int index);
private:
    ///Количество команд
    int     gcode_size;
    ///Строка, из которой сгенерируется G-code
    QString gcode_str;
    ///Главный список команд
    QList<QString>   gcode_list;

};

#endif // GCODE_H
