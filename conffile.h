#ifndef CONFFILE_H
#define CONFFILE_H

#include <QFile>
#include <QList>
#include <QByteArray>
#include <QTextStream>

class ConfFile
{
public:
    ConfFile(const QString&);
    bool read();
    int parametersNumber();
    QString parameter(int);
    bool create();
private:
    QFile           *file;
    QList<QString>  *list;
    QString         *strings;
    bool            first = 1;
};

#endif // CONFFILE_H
