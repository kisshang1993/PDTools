/*************************************************
Copyright: 2018-2019 Melux
Author: ChengHang
Date:2019-01-03
Description: 扫描列表
**************************************************/
#ifndef BOARDCASTLISTWIDGET_H
#define BOARDCASTLISTWIDGET_H

#include <QWidget>
#include <QDebug>
#include <QListWidget>
#include <QUdpSocket>
#include <QCloseEvent>
#include <QSettings>
#include <QTimer>
#include <QMessageBox>
#include "util.h"
#include "pd_Protocol.h"

#define COLOR_SUCCESS QColor(0, 180, 50) //绿色
#define COLOR_FAILED QColor(200, 0, 50) //红色

class BoardCastListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BoardCastListWidget(QWidget *parent = nullptr);
    //析构函数
    ~BoardCastListWidget();
    //初始化列表(构造)
    void initListWidget(bool withCheckState = false);
    //隐藏已连接地址
    void hideConnected(QString currentIP);
    //重新搜索
    void rescanDevices();
    //获取选择列
    QList<QString> getSelectRow(bool getall = false);
    //设置同步结果
    void setSyncResult(int row, QString state);
    //结构体转字节流
    QByteArray structToStream(pd_Package pkg);

signals:
    //单击
    void singleClick(QString ip, int port);
    //双击
    void doubleClick();
    //void doubleClick(QString ip, int port);

public slots:
    //广播设备
    void broadcastDevices();
    //广播设备 载入效果
    void scanChangeSlot();
    //响应设备
    void hasDevicesRespone();
    //扫描列表单击
    void devicesListClicked(const QModelIndex &index);
    //扫描列表双击
    void devicesListDoubleClicked(const QModelIndex &index);

private:
    //绑定状态
    bool bind_state;
    //列表 Wdiget
    QListWidget *devicesList;
    //是否含选择框标记
    bool checkState;
    //隐藏已经连接的主机地址
    bool isHideConnected = false;
    //已连接的主机地址
    QString connectedIP;
    //UDP Socket
    QUdpSocket *broadcastSocket;
    //UDP广播端口
    int broadcastPort;
    //UDP 广播内容
    QByteArray broadcastStr;
    //UDP 广播计时器
    QTimer *broadcastTimer;
    //广播设备 载入效果 计时器
    QTimer *scanChangeTimer;
    //广播设备 载入效果 控件
    QLabel *scanLabel;
    //广播设备 载入效果 信息列表
    QStringList *scanChangeList;
    //检测到的有效设备保存列表
    QList<QString> *detectDevices;
    //广播地址
    QString boardcastAddress;
    //本机IP
    QString configLocalIP;
};

#endif // BOARDCASTLISTWIDGET_H
