#include "toserver.h"
#include "ui_toserver.h"

toserver::toserver(QString NowIP,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::toserver)
{
    ui->setupUi(this);

    ui->lineEdit->setText(NowIP);
}

toserver::~toserver()
{
    delete ui;
}

void toserver::on_buttonBox_accepted()
{
    QString NowHost=ui->lineEdit->text();
    emit StartServer(NowHost);
}

void toserver::on_buttonBox_rejected()
{
    emit CancelServer();
}
