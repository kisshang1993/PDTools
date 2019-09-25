/*************************************************
Copyright: 2018-2019 Melux
Author: ChengHang
Date:2018-05-14
Description: 登录窗口
**************************************************/
#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QProcess>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QProgressDialog>
#include <QMessageBox>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonDocument>
#include "util.h"
#include "pd_Protocol.h"
#include "boardcastlistwidget.h"
#include "inheritclass.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);
    //设置全局TcpSocket
    void setSocket(QTcpSocket *mainSocket);
    //设置版本信息
    void setVersion(int ver);
    //设置退出状态
    void setQuitMode(int mode);
    //连接成功
    void acceptConnect();
    //类析构函数
    ~LoginDialog();


private slots:
    //TCP连接
    void on_connect_btn_clicked();
    //ADB连接
    void on_adbconnectBtn_clicked();
    //广播列表单击
    void on_devicesList_single_clicked(QString ip, int port);

private:
    //UI
    Ui::LoginDialog *ui;
    //ADB连接进度框（循环）
    QProgressDialog *initProDlg;
    //TcpSocket实例
    QTcpSocket *socket;
    //扫描列表实例
    BoardCastListWidget *boardCastListWidget;
    //本机IP
    QString configLocalIP;
};

#endif // LOGINDIALOG_H
