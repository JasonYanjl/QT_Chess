#ifndef TOCLIENT_H
#define TOCLIENT_H

#include <QDialog>
#include<QLineEdit>
#include<QPushButton>

namespace Ui {
class toclient;
}

class toclient : public QDialog
{
    Q_OBJECT

public:
    explicit toclient(QWidget *parent = nullptr);
    ~toclient();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

    void on_pushButton_12_clicked();

    void on_pushButton_13_clicked();

signals:
    void StartClient(QString);

private:
    Ui::toclient *ui;

    QString Nows;
};

#endif // TOCLIENT_H
