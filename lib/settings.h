#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QString>
#include "config.h"
#include <QMessageBox>

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

private:
    Ui::Settings    *ui;
    Config          *config;
};

#endif // SETTINGS_H
