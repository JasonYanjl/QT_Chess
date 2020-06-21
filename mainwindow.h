#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QtNetwork>
#include<QHostAddress>
#include<QtDebug>
#include<QMessageBox>
#include<QPaintEvent>
#include<QPainter>
#include<QMouseEvent>
#include<QLabel>
#include<QInputDialog>
#include<iostream>
#include<QFileDialog>
#include<QTimer>
#include"toserver.h"
#include"toclient.h"

#define MAXWH 10
#define MATRIXWH 60
#define MAXTIME 60

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionAsServer_triggered();

    void on_actionAsClient_triggered();

    void StartServer(QString);
    void CancelServer();
    void StartClient(QString);
    void acceptConnection();
    void connectedToServer();
    void resvMessage();

    void on_actionSurrender_triggered();

    void on_actionSave_triggered();

    void on_actionLoad_triggered();

    void StartTime();

    void DecTime();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);

private:
    Ui::MainWindow *ui;

    QTcpSocket *readWriteSocket;
    QTcpServer *listenSocket;

    bool linked;
    bool isServerOpen,isSocketOpen,isGameOn;
    int myturn,nowturn;

    int Clickx,Clicky;

    int NowTime;

    QTimer *timer;

    std::pair<int,int> Place[MAXWH][MAXWH];
    std::pair<int,int> Piece[MAXWH][MAXWH];

    bool CheckIPAddress(QString IPaddress);
    bool CheckMove(int x1,int y1,int x2,int y2);
    bool CheckMoveWithoutColor(int x1,int y1,int x2,int y2);
    bool CheckEmpty(int x1,int y1,int x2,int y2);
    bool CheckWin();
    bool CheckDraw(int WhichTurn);
    bool CheckKingRook(int x1,int y1,int x2,int y2);
    bool CannotMove(int WhichTurn,int kx,int ky);
    bool BeAttack(int WhichTurn,int x1,int y1);
    void PieceMove(int x1,int y1,int x2,int y2);
    void PieceChange(int x1,int y1,int WhichTurn,int WhatPiece);
    int CheckPieceChange();
    void PreDo();
    void SendMessage(QString NowToSend);
    void readMessage(std::string NowToRead);
};

#endif // MAINWINDOW_H
