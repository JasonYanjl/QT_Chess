#include "toclient.h"
#include "ui_toclient.h"

toclient::toclient(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::toclient)
{
    ui->setupUi(this);

    Nows="";

    ui->lineEdit->setText("");
}

toclient::~toclient()
{
    delete ui;
}

void toclient::on_pushButton_clicked()
{
    Nows+='0';
    ui->lineEdit->setText(Nows);
}

void toclient::on_pushButton_2_clicked()
{
    Nows+='1';
    ui->lineEdit->setText(Nows);
}

void toclient::on_pushButton_3_clicked()
{
    Nows+='2';
    ui->lineEdit->setText(Nows);
}

void toclient::on_pushButton_4_clicked()
{
    Nows+='3';
    ui->lineEdit->setText(Nows);
}

void toclient::on_pushButton_5_clicked()
{
    Nows+='4';
    ui->lineEdit->setText(Nows);
}

void toclient::on_pushButton_6_clicked()
{
    Nows+='5';
    ui->lineEdit->setText(Nows);
}

void toclient::on_pushButton_7_clicked()
{
    Nows+='6';
    ui->lineEdit->setText(Nows);
}

void toclient::on_pushButton_8_clicked()
{
    Nows+='7';
    ui->lineEdit->setText(Nows);
}

void toclient::on_pushButton_9_clicked()
{
    Nows+='8';
    ui->lineEdit->setText(Nows);
}

void toclient::on_pushButton_10_clicked()
{
    Nows+='9';
    ui->lineEdit->setText(Nows);
}

void toclient::on_pushButton_11_clicked()
{
    Nows+='.';
    ui->lineEdit->setText(Nows);
}

void toclient::on_pushButton_12_clicked()
{
    if (Nows.length()>0)
        Nows=Nows.left(Nows.length() - 1);
    ui->lineEdit->setText(Nows);
}

void toclient::on_pushButton_13_clicked()
{
    emit StartClient(ui->lineEdit->text());
    close();
}
