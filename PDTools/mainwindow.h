/*************************************************
Copyright: 2018-2019 Melux
Author: ChengHang
Date:2018-05-14
Description: 主窗口
**************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "pd_Protocol.h"
#include "imagedisplay.h"
#include "boardcastlistwidget.h"
#include "inheritclass.h"
#include "util.h"
#include <QMainWindow>
#include <QProcess>
#include <QDataStream>
#include <QTimer>
#include <QDebug>
#include <QJsonObject>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardItemModel>
#include <QFontDatabase>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QFile>
#include <QLineEdit>
#include <QKeyEvent>
#include <QRadioButton>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDesktopServices>
#include <QSpinBox>
#include <QMap>
#include <QComboBox>
#include <QListView>
#include <QDir>

//高本版VS编译器中文编码支持
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#define SPACING 5 //按钮间隔

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    //窗口重绘
    //void paintEvent(QPaintEvent *);
    //设置套接字
    void setSocket(QTcpSocket *mainSocket, int ver);
    //初始化工具
    void initTools(const QByteArray &initJsonBytes);
    //设置重启状态
    void setRestartState(int &state);
    //生成UI树
    void generateTree(const QByteArray &jsonBytes);
    //传输UIJSON
    void transmissionJSON();
    //传输修改的值
    void transmissionModify(const QStringList &keys, QString value);
    //查找键父级
    QStringList findKey(const QJsonObject &obj, QString &key);
    //查找键注释
    QString findDescForKey(QString &key);
    //修改值
    bool modifyVal(const QStringList &keyList, QString val);
    //结构体转字节流
    QByteArray structToStream(pd_Package pkg);
    //关闭连接
    void connectionClose(int state);
    //同步设备
    void syncDevices(QList<QString> selection);
    //清空模型
    void modelClear();
    //接受数据处理
    void parseRecvData();
    //发送响应
    void sendRespones();
    //发送请求
    void sendRequest(const char *body, COMMAND cmd);
    //发送请求，重写
    void sendRequest(COMMAND cmd);
    //检查返回状态
    bool checkReplySuccess(QByteArray jsonBytes, QString &error);
    //FTP上传
    void uploadByFTP();


    ~MainWindow();

private slots:
    //tree 点击槽

    void treeClicked(const QModelIndex &index);
    //查找Tree 槽函数
    void on_searchTreelineEdit_textChanged(const QString &arg1);
    //table 点击槽
    void tableClicked(const QModelIndex &index);
    //接收数据
    void socketReadData();
    //断开Socket连接 槽函数
    void socketDisconnect();
    //系统信息
    void displaSystemInfo();
    //图像调试控制
    void imageDebugControl(const QString &type, const QString &cmd);
    //退出
    void on_quitWindows_triggered();
    //关闭连接
    void on_closeConnection_triggered();
    //关于QT
    void on_aboutQt_triggered();
    //软件信息
    void on_infomation_triggered();
    //检查更新
    void on_checkUpdate_triggered();
    //配置文件
    void on_action_triggered();
    //等待动画
    void waitLabelTextChange();

    /* 模块 */
    //保存JSON到本地
    void saveJsonToFile();
    //读取JSON到配置
    void loadJsonfromFile();
    //掌脉图像调试
    void on_palmImageDebugbtn_clicked();
    //面部图像调试
    void on_faceImageDebugbtn_clicked();
    //上传文件
    void uploadFirmware();
    //同步配置
    void syncConfigDevices();
    //UIJSON列表选择 事件
    void on_jsonSelectComboBox_currentIndexChanged(const QString &arg1);
    //FTP 上传结束 槽函数
    void replyFTPFinished(QNetworkReply *ftpReply);
    //FTP 上传错误 槽函数
    void loadFTPError(QNetworkReply::NetworkError);
    //FTP 上传进度 槽函数
    void loadFTPProgress(qint64 bytesSent, qint64 bytesTotal);

    void on_action_U_triggered();

protected:
    //重写关闭事件
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
    //版本
    int version = 100;
    //进度框
    QProgressDialog *initProDlg;
    //掌脉图像调试窗口
    ImageDisplay *palmImagePanel = nullptr;
    //面容图像调试窗口
    ImageDisplay *faceImagePanel = nullptr;
    //同步设备窗口
    QDialog *syncWidget = nullptr;
    //同步设备 扫描列表
    BoardCastListWidget *boardCastListWidget = nullptr;
    //TcpSocket
    QTcpSocket *socket;
    //等待接受数据 Label
    QLabel *waitLabel;
    //等待接受数据 信息列表
    QStringList *waitLabelText;
    //等待接受数据 计时器
    QTimer *waitLabelTimer;
    //JSON TREE 模型
    QStandardItemModel *treeModel;
    //JSON TABLE 模型
    QStandardItemModel* model;
    //UIJSON对象
    QJsonObject jsonUiObj;
    //报文结构体
    pd_Package pdPkg;
    //报文命令
    char cmd;
    //单改模式
    bool singleModify = false;
    //保存模式
    bool onlySave = false;
    //接受字节块
    QByteArray tcpRecvBlock;
    //连接状态 Label
    QLabel *connectStatus;
    //内存使用 Label
    QLabel *memoryUsageStatus;
    //刷新内存使用 计时器
    QTimer *memoryUsageTimer;
    //报文长度
    int pkgLen = 0;
    //校验长度
    int crcLen = 0;
    //重启状态
    int *restart;
    //分段读取 头部读取完毕状态
    bool isTcpRecvHeadOk = false;
    //手动关闭Socket标记
    bool isSocketManualClose = false;
    //FTP 对象
    QNetworkReply *ftpReply;
    //待上传的文件路径
    QString selectFilePath;
    //上传固件 进度条
    QProgressBar *uploadFirmwareProgressBar;
    //上传固件 状态
    bool uploadStatus;
    //上传固件 上次选的的路径
    QString uploadBeforePath = "";
    //当前连接
    QString connectedIP;
};

#endif // MAINWINDOW_H
