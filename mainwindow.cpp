#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    linked=0;
    isServerOpen=isSocketOpen=isGameOn=0;

    myturn=0;
    nowturn=-1;

    NowTime=MAXTIME;

    Clickx=Clicky=0;

    timer=new QTimer();
    timer->stop();
    connect(timer,SIGNAL(timeout()),this,SLOT(DecTime()));

    memset(Place,0,sizeof(Place));
    //Place
    for(int i=1;i<=8;i++)
        for(int j=1;j<=8;j++) {
            Place[i][j].first=MATRIXWH*i;
            Place[i][j].second=MATRIXWH * (8-j+1);
        }

    memset(Piece,0,sizeof(Piece));

    ui->lcdNumber->display(NowTime);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionAsServer_triggered()
{
    QString IPaddress=QHostAddress(QHostAddress::LocalHost).toString();
    toserver *Toserver=new toserver(IPaddress);
    connect(Toserver,SIGNAL(StartServer(QString)),this,SLOT(StartServer(QString)));
    connect(Toserver,SIGNAL(CancelServer()),this,SLOT(CancelServer()));
    Toserver->show();
    Toserver->exec();
}

void MainWindow::on_actionAsClient_triggered()
{
    toclient *Toclient=new toclient();
    connect(Toclient,SIGNAL(StartClient(QString)),this,SLOT(StartClient(QString)));
    Toclient->show();
    Toclient->exec();
}

void MainWindow::StartServer(QString IPaddress)
{
    qDebug()<<"StartServer";
    if (!CheckIPAddress(IPaddress)) {
        QMessageBox::critical(NULL, "Warning", "IP address is illegal,Retry",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
    }
    qDebug()<<"StartServer"<<IPaddress;
    listenSocket=new QTcpServer();
    listenSocket->listen(QHostAddress(IPaddress),8888);
    connect(listenSocket,SIGNAL(newConnection()),this,SLOT(acceptConnection()));
    isServerOpen=1;
}

void MainWindow::CancelServer()
{
    qDebug()<<"CancelServer";
    if (isServerOpen) {
        listenSocket->close();
        isServerOpen=0;
        isSocketOpen=0;
        linked=0;
    }
}

void MainWindow::StartClient(QString IPaddress)
{
    qDebug()<<"StartClient"<<IPaddress;
    if (!CheckIPAddress(IPaddress)){
        QMessageBox::critical(NULL, "Warning", "IP address is illegal,Retry",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
    }
    readWriteSocket = new QTcpSocket;
    readWriteSocket->connectToHost(QHostAddress(IPaddress),8888);
    connect(readWriteSocket,SIGNAL(connected()),this,SLOT(connectedToServer()));
    if (!readWriteSocket->waitForConnected(3000)) {
        QMessageBox::critical(NULL, "Warning", "connect fail",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    }
    else {
        QMessageBox::information(NULL, "information", "connect success",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    }
}

void MainWindow::acceptConnection(){
    qDebug()<<"acceptConnection";
    isSocketOpen=1;
    readWriteSocket=listenSocket->nextPendingConnection();
    connect(readWriteSocket,SIGNAL(readyRead()),this,SLOT(resvMessage()));
    //
    linked=1;
    myturn=1;
    PreDo();
}

void MainWindow::connectedToServer()
{
    qDebug()<<"connectedToServer";
    isSocketOpen=1;
    connect(readWriteSocket,SIGNAL(readyRead()),this,SLOT(resvMessage()));
    //
    linked=1;
    myturn=2;
    PreDo();
}

void MainWindow::resvMessage()
{
    QString NowMessage;
    NowMessage="";
    NowMessage+=readWriteSocket->readAll();
    qDebug()<<"resv"<<NowMessage;
    std::string nows=NowMessage.toStdString(),tmps="";
    for(int i=0;i<nows.length();i++) {
        if (nows[i]==';') {
            if (tmps!="") readMessage(tmps);
            tmps="";
        }
        else tmps=tmps+nows[i];
    }
    if (tmps!="") readMessage(tmps);
}

void MainWindow::SendMessage(QString NowToSend)
{
    if (linked==0) return;
    qDebug()<<"Send"<<NowToSend;
    QByteArray *array =new QByteArray;
    array->clear();
    array->append(NowToSend);
    readWriteSocket->write(array->data());
}

void MainWindow::readMessage(std::string NowToRead)
{
    qDebug()<<QString::fromStdString(NowToRead);
    QVector<int> readin;
    readin.clear();
    int tmp=-1;
    std::string NowType="";
    for(int i=0;i<NowToRead.length();i++) {
        if (NowToRead[i]==' ' || NowToRead[i]==';') {
            if (tmp!=-1) readin.push_back(tmp);
            tmp=-1;
        }
        else if ('0'<=NowToRead[i] && NowToRead[i]<='9') {
            if (tmp==-1) tmp=0;
            tmp=tmp * 10+(NowToRead[i]-'0');
        }
        else NowType=NowType+NowToRead[i];
    }
    if (tmp!=-1) readin.push_back(tmp);
    if (NowType=="Move") {
        PieceMove(readin[0],readin[1],readin[2],readin[3]);
        repaint();
    }
    if (NowType=="Change") {
        PieceChange(readin[0],readin[1],readin[2],readin[3]);
        repaint();
    }
    if (NowType=="Lost") {
        isGameOn=0;
        QMessageBox::information(NULL, "Title", "You Lost",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        StartTime();
    }
    if (NowType=="Draw") {
        isGameOn=0;
        QMessageBox::information(NULL, "Title", "Draw",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        StartTime();
    }
    if (NowType=="Win") {
        isGameOn=0;
        QMessageBox::information(NULL, "Title", "You Win",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        StartTime();
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    if (!isGameOn) return;

    if (nowturn==0) ui->label->setText("");
    else if (nowturn==1) ui->label->setText("Now it's the turn of White");
    else if (nowturn==2) ui->label->setText("Now it's the turn of Black");

    QPainter painter(this);

    if (!linked) return;

    //draw rect
    for(int i=1;i<=8;i++)
        for(int j=1;j<=8;j++) {
            painter.save();
            if ((i+j) % 2==0) {
                QBrush brush(QColor(181,135,99));
                painter.setBrush(brush);
                painter.drawRect(Place[i][j].first,Place[i][j].second,MATRIXWH,MATRIXWH);
            }
            else {
                QBrush brush(QColor(240,218,181));
                painter.setBrush(brush);
                painter.drawRect(Place[i][j].first,Place[i][j].second,MATRIXWH,MATRIXWH);
            }
            painter.restore();
        }

    //draw Piece
    for(int i=1;i<=8;i++)
        for(int j=1;j<=8;j++) {
            if (Piece[i][j].first==0) continue;
            if (Piece[i][j].first==1) {
                QPixmap pix;
                if (Piece[i][j].second==1) {
                    pix.load(":/new/photo/photo/white_king.png");
                }
                else if (Piece[i][j].second==2) {
                    pix.load(":/new/photo/photo/white_queen.png");
                }
                else if (Piece[i][j].second==3) {
                    pix.load(":/new/photo/photo/white_rook.png");
                }
                else if (Piece[i][j].second==4) {
                    pix.load(":/new/photo/photo/white_bishop.png");
                }
                else if (Piece[i][j].second==5) {
                    pix.load(":/new/photo/photo/white_knight.png");
                }
                else if (Piece[i][j].second==6) {
                    pix.load(":/new/photo/photo/white_pawn.png");
                }
                painter.drawPixmap(Place[i][j].first,Place[i][j].second,MATRIXWH,MATRIXWH,pix);
            }
            else if (Piece[i][j].first==2) {
                QPixmap pix;
                if (Piece[i][j].second==1) {
                    pix.load(":/new/photo/photo/black_king.png");
                }
                else if (Piece[i][j].second==2) {
                    pix.load(":/new/photo/photo/black_queen.png");
                }
                else if (Piece[i][j].second==3) {
                    pix.load(":/new/photo/photo/black_rook.png");
                }
                else if (Piece[i][j].second==4) {
                    pix.load(":/new/photo/photo/black_bishop.png");
                }
                else if (Piece[i][j].second==5) {
                    pix.load(":/new/photo/photo/black_knight.png");
                }
                else if (Piece[i][j].second==6) {
                    pix.load(":/new/photo/photo/black_pawn.png");
                }
                painter.drawPixmap(Place[i][j].first,Place[i][j].second,MATRIXWH,MATRIXWH,pix);
            }
        }

    //draw Click
    if (Clickx!=0 && Clicky!=0) {
        painter.save();
        QPen pen(Qt::red);
        painter.setPen(pen);
        painter.drawLine(Place[Clickx][Clicky].first+1,Place[Clickx][Clicky].second+1,
                         Place[Clickx][Clicky].first+MATRIXWH-1,Place[Clickx][Clicky].second+1);
        painter.drawLine(Place[Clickx][Clicky].first+1,Place[Clickx][Clicky].second+1,
                         Place[Clickx][Clicky].first+1,Place[Clickx][Clicky].second+MATRIXWH-1);
        painter.drawLine(Place[Clickx][Clicky].first+1,Place[Clickx][Clicky].second+MATRIXWH-1,
                         Place[Clickx][Clicky].first+MATRIXWH-1,Place[Clickx][Clicky].second+MATRIXWH-1);
        painter.drawLine(Place[Clickx][Clicky].first+MATRIXWH-1,Place[Clickx][Clicky].second+1,
                         Place[Clickx][Clicky].first+MATRIXWH-1,Place[Clickx][Clicky].second+MATRIXWH-1);
        painter.restore();
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button()!=Qt::LeftButton) return;
    if (!linked) return;
    if (!isGameOn) return;
    if (myturn==nowturn) {
        int nx=0,ny=0;
        int x=event->x();
        int y=event->y();
        for(int i=1;i<=8;i++) {
            if (nx!=0) break;
            for(int j=1;j<=8;j++) {
                if (Place[i][j].first<=x && x<=Place[i][j].first+MATRIXWH-1
                        && Place[i][j].second<=y && y<=Place[i][j].second+MATRIXWH-1) {
                    nx=i;
                    ny=j;
                    break;
                }
            }
        }
        if (Clickx==nx && Clicky==ny) return;
        if (Clickx==0 || Clicky==0) {
            Clickx=nx;
            Clicky=ny;
            repaint();
            return;
        }
        else {
            if (CheckMove(Clickx,Clicky,nx,ny)) {
                //PieceMove
                PieceMove(Clickx,Clicky,nx,ny);
                int PieceChangeTo=CheckPieceChange();
                SendMessage(tr("Move %1 %2 %3 %4;").arg(Clickx).arg(Clicky).arg(nx).arg(ny));
                if (PieceChangeTo!=0) {
                    PieceChange(nx,ny,myturn,PieceChangeTo);
                    SendMessage(tr("Change %1 %2 %3 %4;").arg(nx).arg(ny).arg(myturn).arg(PieceChangeTo));
                }
                //CheckWin
                if (CheckWin()) {
                    isGameOn=0;
                    QMessageBox::information(NULL, "Title", "You Win",
                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                    SendMessage(tr("Lost"));
                    StartTime();
                    return;
                }
                //CheckDraw
                if (CheckDraw(3-myturn)) {
                    isGameOn=0;
                    QMessageBox::information(NULL, "Title", "Draw",
                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                    SendMessage(tr("Draw"));
                    StartTime();
                    return;
                }
            }
            else if (CheckKingRook(Clickx,Clicky,nx,ny)) {
                if (Clickx==5 && Clicky==1 && nx==7 && ny==1) {
                    PieceMove(5,1,7,1);
                    PieceMove(8,1,6,1);
                    SendMessage(tr("Move %1 %2 %3 %4;").arg(5).arg(1).arg(7).arg(1));
                    SendMessage(tr("Move %1 %2 %3 %4;").arg(8).arg(1).arg(6).arg(1));

                    PieceMove(0,0,0,0);
                    SendMessage(tr("Move %1 %2 %3 %4;").arg(0).arg(0).arg(0).arg(0));
                }
                else if (Clickx==5 && Clicky==1 && nx==3 && ny==1) {
                    PieceMove(5,1,3,1);
                    PieceMove(1,1,4,1);
                    SendMessage(tr("Move %1 %2 %3 %4;").arg(5).arg(1).arg(3).arg(1));
                    SendMessage(tr("Move %1 %2 %3 %4;").arg(1).arg(1).arg(4).arg(1));

                    PieceMove(0,0,0,0);
                    SendMessage(tr("Move %1 %2 %3 %4;").arg(0).arg(0).arg(0).arg(0));
                }
                else if (Clickx==5 && Clicky==8 && nx==7 && ny==8) {
                    PieceMove(5,8,7,8);
                    PieceMove(8,8,6,8);
                    SendMessage(tr("Move %1 %2 %3 %4;").arg(5).arg(8).arg(7).arg(8));
                    SendMessage(tr("Move %1 %2 %3 %4;").arg(8).arg(8).arg(6).arg(8));

                    PieceMove(0,0,0,0);
                    SendMessage(tr("Move %1 %2 %3 %4;").arg(0).arg(0).arg(0).arg(0));
                }
                else if (Clickx==5 && Clicky==8 && nx==3 && ny==8) {
                    PieceMove(5,8,3,8);
                    PieceMove(1,8,4,8);
                    SendMessage(tr("Move %1 %2 %3 %4;").arg(5).arg(8).arg(3).arg(8));
                    SendMessage(tr("Move %1 %2 %3 %4;").arg(1).arg(8).arg(4).arg(8));

                    PieceMove(0,0,0,0);
                    SendMessage(tr("Move %1 %2 %3 %4;").arg(0).arg(0).arg(0).arg(0));
                }
                //CheckWin
                if (CheckWin()) {
                    isGameOn=0;
                    QMessageBox::information(NULL, "Title", "You Win",
                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                    SendMessage(tr("Lost"));
                    StartTime();
                    return;
                }
                //CheckDraw
                if (CheckDraw(3-myturn)) {
                    isGameOn=0;
                    QMessageBox::information(NULL, "Title", "Draw",
                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                    SendMessage(tr("Draw"));
                    StartTime();
                    return;
                }
            }
            Clickx=nx;
            Clicky=ny;
            repaint();
        }
    }
    else {
        int x=event->x();
        int y=event->y();
        for(int i=1;i<=8;i++)
            for(int j=1;j<=8;j++) {
                if (Place[i][j].first<=x && x<=Place[i][j].first+MATRIXWH-1
                        && Place[i][j].second<=y && y<=Place[i][j].second+MATRIXWH-1) {
                    Clickx=i;
                    Clicky=j;
                    repaint();
                    return;
                }
            }
    }
}

bool MainWindow::CheckIPAddress(QString IPaddress)
{
    std::string tmps= IPaddress.toStdString();
    int now=0;
    int totpoi=0;
    int length=tmps.length();
    for(int i=0;i<length;i++) {
        if (tmps[i]=='.') {
            if (now<0 || now>255) return 0;
            if (i==0) return 0;
            if (tmps[i-1]=='.') return 0;
            now=0;
            totpoi++;
        }
        else if ('0'<=tmps[i] && tmps[i]<='9'){
            now=now * 10+(tmps[i]-'0');
        }
    }
    if (now<0 || now>255) return 0;
    if (totpoi!=3) return 0;
    return 1;
}

bool MainWindow::CheckMove(int x1, int y1, int x2, int y2)
{
    if (linked==0) return 0;
    if (x1<1 || x1>8 || y1<1 || y1>8) return 0;
    if (x2<1 || x2>8 || y2<1 || y2>8) return 0;
    if (Piece[x1][y1].first!=myturn) return 0;
    if (Piece[x2][y2].first==myturn) return 0;
    if (x1==x2 && y1==y2) return 0;
    if (Piece[x1][y1].second==1) {
        if (qAbs(x1-x2)<=1 && qAbs(y1-y2)<=1) return 1;
        return 0;
    }
    else if (Piece[x1][y1].second==2) { // queen
        if (x1==x2 || y1==y2 || x1-y1==x2-y2 || x1+y1==x2+y2) {
            if (CheckEmpty(x1,y1,x2,y2)) return 1;
            else return 0;
        }
        else return 0;
    }
    else if (Piece[x1][y1].second==3) { //rook
        if (x1==x2 || y1==y2) {
            if (CheckEmpty(x1,y1,x2,y2)) return 1;
            else return 0;
        }
        else return 0;
    }
    else if (Piece[x1][y1].second==4) { //bishop
        if (x1-y1==x2-y2 || x1+y1==x2+y2) {
            if (CheckEmpty(x1,y1,x2,y2)) return 1;
            else return 0;
        }
        else return 0;
    }
    else if (Piece[x1][y1].second==5) { //knight
        if (qAbs(x1-x2)==1 && qAbs(y1-y2)==2) return 1;
        if (qAbs(x1-x2)==2 && qAbs(y1-y2)==1) return 1;
        return 0;
    }
    else if (Piece[x1][y1].second==6) {
        if (Piece[x1][y1].first==1) {
            if (x1==x2) {
                if (y2==y1+1) {
                    if (CheckEmpty(x1,y1,x1,y2+1)==0) return 0;
                    else return 1;
                }
                else if (y2==y1+2) {
                    if (CheckEmpty(x1,y1,x1,y2+1)==0) return 0;
                    if (y1==2) return 1;
                    else return 0;
                }
            }
            else {
                if (qAbs(x1-x2)!=1) return 0;
                if (y2-y1!=1) return 0;
                if (Piece[x2][y2].first!=2) return 0;
                return 1;
            }
        }
        else if (Piece[x1][y1].first==2) {
            if (x1==x2) {
                if (y2==y1-1) {
                    if (CheckEmpty(x1,y1,x1,y2-1)==0) return 0;
                    else return 1;
                }
                else if (y2==y1-2) {
                    if (CheckEmpty(x1,y1,x1,y2-1)==0) return 0;
                    if (y1==7) return 1;
                    else return 0;
                }
            }
            else {
                if (qAbs(x1-x2)!=1) return 0;
                if (y1-y2!=1) return 0;
                if (Piece[x2][y2].first!=1) return 0;
                return 1;
            }
        }
        return 0;
    }
    return 0;
}

bool MainWindow::CheckMoveWithoutColor(int x1, int y1, int x2, int y2)
{
    if (linked==0) return 0;
    if (x1<1 || x1>8 || y1<1 || y1>8) return 0;
    if (x2<1 || x2>8 || y2<1 || y2>8) return 0;
    if (Piece[x2][y2].first==Piece[x1][y1].first) return 0;
    if (x1==x2 && y1==y2) return 0;
    if (Piece[x1][y1].second==1) {
        if (qAbs(x1-x2)<=1 && qAbs(y1-y2)<=1) return 1;
        return 0;
    }
    else if (Piece[x1][y1].second==2) { // queen
        if (x1==x2 || y1==y2 || x1-y1==x2-y2 || x1+y1==x2+y2) {
            if (CheckEmpty(x1,y1,x2,y2)) return 1;
            else return 0;
        }
        else return 0;
    }
    else if (Piece[x1][y1].second==3) { //rook
        if (x1==x2 || y1==y2) {
            if (CheckEmpty(x1,y1,x2,y2)) return 1;
            else return 0;
        }
        else return 0;
    }
    else if (Piece[x1][y1].second==4) { //bishop
        if (x1-y1==x2-y2 || x1+y1==x2+y2) {
            if (CheckEmpty(x1,y1,x2,y2)) return 1;
            else return 0;
        }
        else return 0;
    }
    else if (Piece[x1][y1].second==5) { //knight
        if (qAbs(x1-x2)==1 && qAbs(y1-y2)==2) return 1;
        if (qAbs(x1-x2)==2 && qAbs(y1-y2)==1) return 1;
        return 0;
    }
    else if (Piece[x1][y1].second==6) {
        if (Piece[x1][y1].first==1) {
            if (x1==x2) {
                if (y2==y1+1) {
                    if (CheckEmpty(x1,y1,x1,y2+1)==0) return 0;
                    else return 1;
                }
                else if (y2==y1+2) {
                    if (CheckEmpty(x1,y1,x1,y2+1)==0) return 0;
                    if (y1==2) return 1;
                    else return 0;
                }
            }
            else {
                if (qAbs(x1-x2)!=1) return 0;
                if (y2-y1!=1) return 0;
                if (Piece[x2][y2].first!=2) return 0;
                return 1;
            }
        }
        else if (Piece[x1][y1].first==2) {
            if (x1==x2) {
                if (y2==y1-1) {
                    if (CheckEmpty(x1,y1,x1,y2-1)==0) return 0;
                    else return 1;
                }
                else if (y2==y1-2) {
                    if (CheckEmpty(x1,y1,x1,y2-1)==0) return 0;
                    if (y1==7) return 1;
                    else return 0;
                }
            }
            else {
                if (qAbs(x1-x2)!=1) return 0;
                if (y1-y2!=1) return 0;
                if (Piece[x2][y2].first!=1) return 0;
                return 1;
            }
        }
        return 0;
    }
    return 0;
}

bool MainWindow::CheckKingRook(int x1, int y1, int x2, int y2)
{
    if (linked==0) return 0;
    if (x1<1 || x1>8 || y1<1 || y1>8) return 0;
    if (x2<1 || x2>8 || y2<1 || y2>8) return 0;
    if (Piece[x1][y1].first!=myturn) return 0;
    if (Piece[x2][y2].first==myturn) return 0;
    if (x1==x2 && y1==y2) return 0;
    if (myturn==1) {
        if (x1==5 && y1==1 && x2==7 && y2==1) {
            if (CheckEmpty(5,1,8,1)==0) return 0;
            if (Piece[8][1]!=std::make_pair(1,3)) return 0;
            if (BeAttack(myturn,5,1)) return 0;
            if (BeAttack(myturn,6,1)) return 0;
            if (BeAttack(myturn,7,1)) return 0;
            return 1;
        }
        if (x1==5 && y1==1 && x2==3 && y2==1) {
            if (CheckEmpty(5,1,1,1)==0) return 0;
            if (Piece[1][1]!=std::make_pair(1,3)) return 0;
            if (BeAttack(myturn,5,1)) return 0;
            if (BeAttack(myturn,4,1)) return 0;
            if (BeAttack(myturn,3,1)) return 0;
            return 1;
        }
    }
    else if (myturn==2) {
        if (x1==5 && y1==8 && x2==7 && y2==8) {
            if (CheckEmpty(5,8,8,8)==0) return 0;
            if (Piece[8][8]!=std::make_pair(2,3)) return 0;
            if (BeAttack(myturn,5,8)) return 0;
            if (BeAttack(myturn,6,8)) return 0;
            if (BeAttack(myturn,7,8)) return 0;
            return 1;
        }
        if (x1==5 && y1==8 && x2==3 && y2==8) {
            if (CheckEmpty(5,8,1,8)==0) return 0;
            if (Piece[1][8]!=std::make_pair(2,3)) return 0;
            if (BeAttack(myturn,5,8)) return 0;
            if (BeAttack(myturn,4,8)) return 0;
            if (BeAttack(myturn,3,8)) return 0;
            return 1;
        }
    }
    return 0;
}

bool MainWindow::BeAttack(int WhichTurn, int x1, int y1)
{
    for(int i=1;i<=8;i++)
        for(int j=1;j<=8;j++) {
            if (Piece[i][j].first!=3-WhichTurn) continue;
            int x2=i;
            int y2=j;
            if (Piece[i][j].second==1) {
                if (qAbs(i-x1)<=1 && qAbs(j-y1)<=1) return 1;
            }
            else if (Piece[i][j].second==2) {
                if (x1==x2 || y1==y2 || x1-y1==x2-y2 || x1+y1==x2+y2) {
                    if (CheckEmpty(x1,y1,x2,y2)) return 1;
                }
            }
            else if (Piece[i][j].second==3) {
                if (x1==x2 || y1==y2) {
                    if (CheckEmpty(x1,y1,x2,y2)) return 1;
                }
            }
            else if (Piece[i][j].second==4) {
                if (x1-y1==x2-y2 || x1+y1==x2+y2) {
                    if (CheckEmpty(x1,y1,x2,y2)) return 1;
                }
            }
            else if (Piece[i][j].second==5) {
                if (qAbs(x1-x2)==1 && qAbs(y1-y2)==2) return 1;
                if (qAbs(x1-x2)==2 && qAbs(y1-y2)==1) return 1;
            }
            else if (Piece[i][j].second==6) {
                if (Piece[x2][y2].first==1) {
                    if (qAbs(x1-x2)==1 && y1==y2+1) return 1;
                }
                else if (Piece[x2][y2].first==2) {
                    if (qAbs(x1-x2)==1 && y1+1==y2) return 1;
                }
            }
        }
    return 0;
}

bool MainWindow::CheckEmpty(int x1, int y1, int x2, int y2)
{
    if (x1==x2) {
        for(int i=qMin(y1,y2)+1;i<=qMax(y1,y2)-1;i++)
            if (Piece[x1][i].first!=0) return 0;
        return 1;
    }
    if (y1==y2) {
        for(int i=qMin(x1,x2)+1;i<=qMax(x1,x2)-1;i++)
            if (Piece[i][y1].first!=0) return 0;
        return 1;
    }
    if (x1-y1==x2-y2) {
        for(int i=qMin(x1,x2)+1;i<=qMax(x1,x2)-1;i++)
            if (Piece[i][i-(x1-y1)].first!=0) return 0;
        return 1;
    }
    if (x1+y1==x2+y2) {
        for(int i=qMin(x1,x2)+1;i<=qMax(x1,x2)-1;i++)
            if (Piece[i][(x1+y1)-i].first!=0) return 0;
        return 1;
    }
    return 0;
}

bool MainWindow::CheckWin()
{
    for(int i=1;i<=8;i++)
        for(int j=1;j<=8;j++)
            if (Piece[i][j].first==3-myturn && Piece[i][j].second==1) return 0;
    return 1;
}

bool MainWindow::CheckDraw(int WhichTurn)
{
    int kx=-1,ky=-1;
    for(int i=1;i<=8;i++) {
        if (kx!=-1) break;
        for(int j=1;j<=8;j++)
            if (Piece[i][j].first==WhichTurn && Piece[i][j].second==1) {
                kx=i;
                ky=j;
                break;
            }
    }
    if (kx==-1 && ky==-1) return 0;
    if (!CannotMove(WhichTurn,kx,ky)) return 0;
    for(int i=1;i<=8;i++)
        for(int j=1;j<=8;j++) {
            if (Piece[i][j].first==WhichTurn && Piece[i][j].second==1) {
                if (BeAttack(WhichTurn,i,j)) return 0;
                for(int k1=-1;k1<=1;k1++)
                    for(int k2=-1;k2<=1;k2++) {
                        if (k1==0 && k2==0) continue;
                        int xx=i+k1,yy=j+k2;
                        if (xx<1 || xx>8 || yy<1 || yy>8) continue;
                        if (Piece[xx][yy].first==WhichTurn) continue;
                        if (BeAttack(WhichTurn,xx,yy)==0) return 0;
                    }
            }
        }
    return 1;
}

bool MainWindow::CannotMove(int WhichTurn,int kx,int ky)
{
    for(int i=1;i<=8;i++)
        for(int j=1;j<=8;j++) {
            if (Piece[i][j].first!=WhichTurn) continue;
            if (Piece[i][j].second==1) continue;
            std::pair<int,int> tmpp=Piece[i][j];
            Piece[i][j]=std::make_pair(0,0);
            if (BeAttack(WhichTurn,kx,ky)) {
                Piece[i][j]=tmpp;
                continue;
            }
            Piece[i][j]=tmpp;
            for(int x=1;x<=8;x++)
                for(int y=1;y<=8;y++) {
                    if (x==i && y==j) continue;
                    if (CheckMoveWithoutColor(i,j,x,y)) return 0;
                }
        }
    return 1;
}

void MainWindow::PieceMove(int x1, int y1, int x2, int y2)
{
    nowturn=3-nowturn;
    StartTime();
    if (x1==0 && y1==0 && x2==0 && y2==0) return;
    Piece[x2][y2]=Piece[x1][y1];
    Piece[x1][y1]=std::make_pair(0,0);
}

void MainWindow::PieceChange(int x1, int y1, int WhichTurn, int WhatPiece)
{
    Piece[x1][y1]=std::make_pair(WhichTurn,WhatPiece);
}

int MainWindow::CheckPieceChange()
{
    QStringList items;
    items.clear();
    items << tr("Queen") << tr("Rook") << tr("Bishop") << tr("Knight");
    if (myturn==1) {
        for(int i=1;i<=8;i++)
            if (Piece[i][8].first==1 && Piece[i][8].second==6) {
                bool ok;
                QString item = QInputDialog::getItem(this, tr("GetYouChoice"),
                                                     tr("Change to:"), items, 0, false, &ok);
                if (item==tr("Queen")) return 2;
                if (item==tr("Rook")) return 3;
                if (item==tr("Bishop")) return 4;
                if (item==tr("Knight")) return 5;
                return 2;
            }
    }
    else if (myturn==2) {
        for(int i=1;i<=8;i++)
            if (Piece[i][1].first==2 && Piece[i][1].second==6) {
                bool ok;
                QString item = QInputDialog::getItem(this, tr("GetYouChoice"),
                                                     tr("Change to:"), items, 0, false, &ok);
                if (item==tr("Queen")) return 2;
                if (item==tr("Rook")) return 3;
                if (item==tr("Bishop")) return 4;
                if (item==tr("Knight")) return 5;
                return 2;
            }
    }
    return 0;
}

void MainWindow::PreDo()
{
    memset(Piece,0,sizeof(Piece));
    //Piece
    Piece[5][1]=std::make_pair(1,1);
    Piece[4][1]=std::make_pair(1,2);
    Piece[3][1]=Piece[6][1]=std::make_pair(1,4);
    Piece[2][1]=Piece[7][1]=std::make_pair(1,5);
    Piece[1][1]=Piece[8][1]=std::make_pair(1,3);
    for(int i=1;i<=8;i++)
        Piece[i][2]=std::make_pair(1,6);
    //
    Piece[5][8]=std::make_pair(2,1);
    Piece[4][8]=std::make_pair(2,2);
    Piece[3][8]=Piece[6][8]=std::make_pair(2,4);
    Piece[2][8]=Piece[7][8]=std::make_pair(2,5);
    Piece[1][8]=Piece[8][8]=std::make_pair(2,3);
    for(int i=1;i<=8;i++)
        Piece[i][7]=std::make_pair(2,6);
    //
    nowturn=1;
    isGameOn=1;
    StartTime();
}

void MainWindow::on_actionSurrender_triggered()
{
    isGameOn=0;
    QMessageBox::information(NULL, "Title", "You Lost",
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    SendMessage(tr("Win"));
    StartTime();
}

void MainWindow::on_actionSave_triggered()
{
    QString filename=QFileDialog::getSaveFileName(this,tr("Save As"));
    if (filename.isEmpty()) return;
    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("TiTle"),
                           tr("cannot write into file %1：/n %2")
                          .arg(filename).arg(file.errorString()));
        return;
    }
    QVector<std::pair<int,int> > v;
    QTextStream out(&file);
    if (nowturn==1) {
        out<<"white"<<"\n";
        for(int k=1;k<=6;k++) {
            v.clear();
            for(int i=1;i<=8;i++)
                for(int j=1;j<=8;j++) {
                    if (Piece[i][j].first==1 && Piece[i][j].second==k)
                        v.push_back(std::make_pair(i,j));
                }
            if (v.size()==0) continue;
            if (k==1) out<<"king ";
            else if (k==2) out<<"queen ";
            else if (k==3) out<<"rook ";
            else if (k==4) out<<"bishop ";
            else if (k==5) out<<"knight ";
            else if (k==6) out<<"pawn ";
            out<<v.size();
            for(int i=0;i<v.size();i++)
                out<<" "<<char('a'+v[i].first-1)<<v[i].second;
            out<<"\n";
        }
        out<<"black"<<"\n";
        for(int k=1;k<=6;k++) {
            v.clear();
            for(int i=1;i<=8;i++)
                for(int j=1;j<=8;j++) {
                    if (Piece[i][j].first==2 && Piece[i][j].second==k)
                        v.push_back(std::make_pair(i,j));
                }
            if (v.size()==0) continue;
            if (k==1) out<<"king ";
            else if (k==2) out<<"queen ";
            else if (k==3) out<<"rook ";
            else if (k==4) out<<"bishop ";
            else if (k==5) out<<"knight ";
            else if (k==6) out<<"pawn ";
            out<<v.size();
            for(int i=0;i<v.size();i++)
                out<<" "<<char('a'+v[i].first-1)<<v[i].second;
            out<<"\n";
        }
    }
    else if (nowturn==2){
        out<<"black"<<"\n";
        for(int k=1;k<=6;k++) {
            v.clear();
            for(int i=1;i<=8;i++)
                for(int j=1;j<=8;j++) {
                    if (Piece[i][j].first==2 && Piece[i][j].second==k)
                        v.push_back(std::make_pair(i,j));
                }
            if (v.size()==0) continue;
            if (k==1) out<<"king ";
            else if (k==2) out<<"queen ";
            else if (k==3) out<<"rook ";
            else if (k==4) out<<"bishop ";
            else if (k==5) out<<"knight ";
            else if (k==6) out<<"pawn ";
            out<<v.size();
            for(int i=0;i<v.size();i++)
                out<<" "<<char('a'+v[i].first-1)<<v[i].second;
            out<<"\n";
        }
        out<<"white"<<"\n";
        for(int k=1;k<=6;k++) {
            v.clear();
            for(int i=1;i<=8;i++)
                for(int j=1;j<=8;j++) {
                    if (Piece[i][j].first==1 && Piece[i][j].second==k)
                        v.push_back(std::make_pair(i,j));
                }
            if (v.size()==0) continue;
            if (k==1) out<<"king ";
            else if (k==2) out<<"queen ";
            else if (k==3) out<<"rook ";
            else if (k==4) out<<"bishop ";
            else if (k==5) out<<"knight ";
            else if (k==6) out<<"pawn ";
            out<<v.size();
            for(int i=0;i<v.size();i++)
                out<<" "<<char('a'+v[i].first-1)<<v[i].second;
            out<<"\n";
        }
    }
}

void MainWindow::on_actionLoad_triggered()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,tr("Input File"),"",tr("*.txt"));
    qDebug()<<fileName;

    if (!fileName.isNull()){
        QFile file(fileName);
        if(!file.open((QFile::ReadOnly|QFile::Text))){
            QMessageBox::warning(this,tr("Error"),tr("Cannot Read File:&1").arg(file.errorString()));
            return;
        }
        QTextStream in(&file);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QString tmp="";
        while(!in.atEnd())
        {
            tmp=tmp+in.readLine()+'\n';
        }
        QApplication::restoreOverrideCursor();
        qDebug()<<tmp;
        std::string nows=tmp.toStdString();
        std::string ss="";
        memset(Piece,0,sizeof(Piece));
        isGameOn=1;
        int thisturn=-1;

        int tmpturn=0;
        int tmppiece=0;
        for(int i=0;i<nows.length();i++) {
            if (('a'<=nows[i] && nows[i]<='z') || ('A'<=nows[i] && nows[i]<='Z') ||
                    ('0'<=nows[i] && nows[i]<='9')) {
                ss=ss+nows[i];
            }
            else {
                if (ss=="") continue;
                if (ss=="white") {
                    tmpturn=1;
                    if (thisturn==-1) thisturn=tmpturn;
                }
                else if (ss=="black") {
                    tmpturn=2;
                    if (thisturn==-1) thisturn=tmpturn;
                }
                else if (ss=="king"){
                    tmppiece=1;
                }
                else if (ss=="queen") {
                    tmppiece=2;
                }
                else if (ss=="rook") {
                    tmppiece=3;
                }
                else if (ss=="bishop"){
                    tmppiece=4;
                }
                else if (ss=="knight"){
                    tmppiece=5;
                }
                else if (ss=="pawn"){
                    tmppiece=6;
                }
                else if (ss.length()==2 && 'a'<=ss[0]<='z' && '0'<=ss[1]<='9'){
                    int x1=ss[0]-'a'+1,y1=ss[1]-'0';
                    if (tmpturn!=0 && tmppiece!=0) Piece[x1][y1]=std::make_pair(tmpturn,tmppiece);
                }
                ss="";
            }
        }
        nowturn=thisturn;
        repaint();
        if (myturn==nowturn && CheckDraw(nowturn)) {
            isGameOn=0;
            QMessageBox::information(NULL, "Title", "Draw",
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            SendMessage(tr("Draw"));
        }
        StartTime();
    }
}

void MainWindow::StartTime()
{
    timer->stop();
    NowTime=MAXTIME;
    ui->lcdNumber->display(NowTime);
    if (isGameOn==0) return;
    if (linked==0) return;
    if (myturn!=nowturn) return;
    timer->start(1000);
}

void MainWindow::DecTime()
{
    NowTime--;
    ui->lcdNumber->display(NowTime);
    if (NowTime==0) {
        QMessageBox::information(NULL, "information", "You Lost",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        SendMessage("Win");
        timer->stop();
        isGameOn=0;
    }
    else {
        timer->start(1000);
    }
}
