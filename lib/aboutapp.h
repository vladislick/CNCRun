#ifndef ABOUTAPP_H
#define ABOUTAPP_H

#include <QDialog>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsScene>

#define APPVERSION "1.2"

namespace Ui {
class AboutApp;
}

class AboutApp : public QDialog
{
    Q_OBJECT

public:
    explicit AboutApp(QWidget *parent = nullptr);
    ~AboutApp();

private slots:
    void on_pushButton_clicked();

private:
    Ui::AboutApp    *ui;
    QGraphicsScene  *scene;
    QPixmap         *imageLogo;
};

#endif // ABOUTAPP_H
