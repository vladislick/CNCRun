#ifndef CONFIG_H
#define CONFIG_H

#include <QFile>
#include <QList>
#include <QByteArray>
#include <QTextStream>

class Config
{
public:
    Config(const QString&);
    //Прочитать название параметра
    QString parameter(int);
    //Прочитать значение параметра
    QString value(int);
    //Прочитать файл
    bool read();
    //Создать файл по экземпляру
    void make(QString);
    //Существует ли файл настроек
    bool isexist();
    //Возвращает количество элементов
    int count();
private:
    QFile           *file;
    QList<QString>  *data;
};

#endif // CONFIG_H
