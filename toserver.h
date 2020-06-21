#ifndef TOSERVER_H
#define TOSERVER_H

#include <QDialog>
#include<QLineEdit>

namespace Ui {
class toserver;
}

class toserver : public QDialog
{
    Q_OBJECT

public:
    explicit toserver(QString NowIP,QWidget *parent = nullptr);
    ~toserver();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

signals:
    void StartServer(QString);
    void CancelServer();

private:
    Ui::toserver *ui;
};

#endif // TOSERVER_H
