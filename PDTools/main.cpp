 /*************************************************
  Copyright: Copyright: 2018-2019 Melux
  Author: ChengHang
  Date: 2018-05-14
  Version: 2.0.0
  Description: 入口函数
**************************************************/

#include "mainwindow.h"
#include "logindialog.h"
#include <QHostAddress>
#include <QApplication>
#include <QSharedMemory>

//高版本VS设置中文编码
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

/*************************************************
    Function:       main
    Description:    程序入口
    Input:          int argc
                    char *argv[]
*************************************************/
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //申请共享内存，应用程序唯一标识
//    QSharedMemory singleton(a.applicationName());
//    if(!singleton.create(1))
//    {
//        QMessageBox::warning(NULL, "警告", "PDTools当前已经有一个实例正在运行！");
//        return false;
//    }
    //全局字体
    QFont font;
    font.setFamily("Microsoft YaHei");
    qApp->setFont(font);
    //退出标记
    int quitState, quitMode = 0;
    //检查更新
    Util::checkUpdateByClient("PDTools");
    int version = Util::getVersionFromDAT();
start:
    //实例化窗口
    quitState = 0;
    LoginDialog loginDialog;
    MainWindow w;
    //全局TcpSocket
    QTcpSocket *socket = new QTcpSocket;
    loginDialog.setSocket(socket);
    loginDialog.setVersion(version);
    loginDialog.setWindowIcon(QIcon(":/pixmap/images/favicon.ico"));
    loginDialog.setWindowTitle("PDTools");
    loginDialog.setQuitMode(quitMode);

    quitMode = 0;
    int loginState = loginDialog.exec();
    //连接状态
    if(loginState)
    {
        w.setWindowTitle("PDTools "+Util::ver2str(version));
        w.setWindowIcon(QIcon(":/pixmap/images/favicon.ico"));
        w.setSocket(socket, version);
        w.setRestartState(quitState);

        a.exec();
    }
    if(quitState==0){
        a.quit();
        return 0;

    }else{

        if(quitState>0) {
            quitMode = quitState;
            quitState = 0;
            goto start;
        }
        return a.exec();
    }
    if(socket != nullptr) delete socket;
    return 0;

}
