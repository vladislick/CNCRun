#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include "conffile.h"
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

private:
    Ui::Settings *ui;
    ConfFile     *file;
};

#endif // SETTINGS_H
