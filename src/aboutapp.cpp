#include "lib/aboutapp.h"
#include "ui_aboutapp.h"

AboutApp::AboutApp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutApp)
{
    ui->setupUi(this);

    /* Указываем иконку */
    this->setWindowIcon(QIcon("cncrun.png"));

    /* Выводим логотип */
    imageLogo = new QPixmap("cncrun.png");
    ui->labelLogo->setPixmap(*imageLogo);

    /* Выводим ссылку проекта */
    ui->labelLink->setText("<a href=\"https://github.com/vladislick/cnc_run_qt\">https://github.com/vladislick/cnc_run_qt</a>");
    ui->labelLink->setOpenExternalLinks(true);

    /* Выводим версию приложения */
    ui->appVersion->setText(APPVERSION);
}

AboutApp::~AboutApp()
{
    delete ui;
}

void AboutApp::on_pushButton_clicked()
{
    this->close();
}
