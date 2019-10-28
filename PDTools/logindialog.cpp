/*************************************************
    Copyright: 2018-2019 Melux
    Author: ChengHang
    Date:2018-05-14
    Description: 登录窗口
**************************************************/
#include "logindialog.h"
#include "ui_logindialog.h"

/**
 * @brief 构造函数
 * @param parent
 */
LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    //读取配置文件
    QFile cfg("config.ini");
    if(!cfg.exists())
    {
        QMessageBox::critical(NULL, "错误", "缺失配置文件！请检查目录下是否存在config.ini");
        return;
    }
    QSettings *config = new QSettings("config.ini", QSettings::IniFormat);
    configLocalIP = config->value("Global/localIP").toString();

    //初始化部分元素
    TitleBar *titleBar = new TitleBar(this);
    ui->timeoutLabel->setHidden(true);
    ui->port_input->setValidator(new QIntValidator(1, 65535, this));
    socket = new QTcpSocket(this);
    QFont font ("Microsoft YaHei", 20, 25);
    ui->title->setFont(font);

    //渲染图像
    ui->logo->setPixmap(QPixmap::fromImage(Util::ScaleImage2Label(QImage(":/pixmap/images/meluxlogo.png"), ui->logo)));
    ui->airwave_logo->setPixmap(QPixmap::fromImage(Util::ScaleImage2Label(QImage(":/pixmap/images/airwave.png"), ui->airwave_logo)));
    ui->visionx_logo->setPixmap(QPixmap::fromImage(Util::ScaleImage2Label(QImage(":/pixmap/images/visionx.png"), ui->visionx_logo)));
    //实例化扫描列表
    boardCastListWidget = new BoardCastListWidget(this);

    boardCastListWidget->setGeometry(ui->groupBox->geometry().x(),
                                     ui->groupBox->geometry().y()+13,
                                     ui->groupBox->geometry().width(),
                                     ui->groupBox->geometry().height()-13);
    boardCastListWidget->initListWidget();
    connect(boardCastListWidget, &BoardCastListWidget::singleClick, this, &LoginDialog::on_devicesList_single_clicked);
    //connect(boardCastListWidget, &BoardCastListWidget::doubleClick, this, &LoginDialog::on_connect_btn_clicked);
    //ui->connect_btn->setDisabled(true);
    //ui->adbconnectBtn->setDisabled(true);
    ui->mac->setText(QString("MAC: %1").arg(Util::getHostMacAddress()));
    ui->local_ip->setText(QString("IP: %1").arg(configLocalIP == "" ? Util::getHostIpAddress() : configLocalIP));

#if TESTMODE > 0
    ui->title->setText("DEBUG MODE");
    ui->title->setStyleSheet("color: #F00");
#endif
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(200);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start();
}

/**
 * @brief 重绘
 */
void LoginDialog::paintEvent(QPaintEvent *)
{
    //绘制阴影
    QPainter painter(this);
    QColor color(0, 0, 0, 0);
    int shadowWidth = 5;
    for(int i=0; i<shadowWidth; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(shadowWidth-i, shadowWidth-i, this->width()-(shadowWidth-i)*2, this->height()-(shadowWidth-i)*2);
        color.setAlpha(30 - i*5);
        painter.setPen(color);
        painter.drawPath(path);
    }
    painter.fillRect(shadowWidth, shadowWidth, width()-shadowWidth*2,height()-shadowWidth*2, QColor(255, 255, 255));

}

/**
 * @brief 引用全局Socket
 * @param mainSocket
 */
void LoginDialog::setSocket(QTcpSocket *mainSocket)
{
    socket = mainSocket;
}

/**
 * @brief 用来UI显示版本号信息
 * @param ver 版本号
 */
void LoginDialog::setVersion(int ver)
{
    ui->ver->setText(QString("ver %1").arg(Util::ver2str(ver)));
}

/**
 * @brief 中断连接程序的退出状态
 * @param mode
 */
void LoginDialog::setQuitMode(int quitMode)
{
    ui->timeoutLabel->setHidden(quitMode == 0);
}

/**
 * @brief 当Socket成功连接至设备,许可下一步
 */
void LoginDialog::acceptConnect()
{
    delete boardCastListWidget;
    this->accept();
}

/**
 * @brief 清理指针
 */
LoginDialog::~LoginDialog()
{
    delete ui;
}

/**
 * @brief 获取窗口IP与PORT发起连接,检查连接状态
 */
void LoginDialog::on_connect_btn_clicked()
{
#if TESTMODE > 0
    acceptConnect();
    return;
#endif
    socket->connectToHost(ui->ip_input->text(), ui->port_input->text().toInt());
    if(!socket->waitForConnected(2000))
    {
        QMessageBox::critical(this, "error", "TCP连接失败，请重试\n"+socket->errorString());
        boardCastListWidget->rescanDevices();
        return;
    }
    qDebug() << QString("connected to %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
    acceptConnect();
}

/**
 * @brief 获取窗口IP与PORT发起连接,检查连接状态
 * @param ip
 * @param port
 */
void LoginDialog::on_devicesList_single_clicked(QString ip, int port)
{
    //boardCastListWidget->rescanDevices();
    ui->ip_input->setText(ip); //ip输入框设置ip
    ui->port_input->setText(QString::number(port));//port输入框设置port
}

/**
 * @brief 调用ADB连接设备模拟TCP
 */
void LoginDialog::on_adbconnectBtn_clicked()
{
    //进度
    QProgressDialog *proDlg = new QProgressDialog(this);
    proDlg->setRange(0, 0);
    proDlg->setWindowTitle("请求数据");
    proDlg->setLabelText("正在初始化程序...");
    proDlg->show();
    //ADB File
    QFile adbFile("adb/adb.exe");
    if(!adbFile.exists())
    {
        QMessageBox::critical(this, "ADB错误", "ADB启程序不存在！请检查程序根目录-adb/adb.exe是否存在。");
        return;
    }
    //启用进程去调用cmd命令模拟TCP
    QProcess adb;
    adb.start("adb/adb.exe forward tcp:18001 tcp:6666");
    if (!adb.waitForStarted()) {
        QMessageBox::critical(this, "ADB连接失败", "Failed to forward C++ debugging ports.");
        return;
    }
    if (!adb.waitForFinished(10000)) {
        QMessageBox::critical(this, "ADB连接失败", "Failed to forward C++ debugging ports.");
        return;
    }
    socket->connectToHost("127.0.0.1", 18001);
    if(!socket->waitForConnected(3000))
    {
        QMessageBox::critical(this, "error", "ADB连接失败！");
        proDlg->close();
        return;
    }
    qDebug() << "connected to ADB";
    //连接成功
    acceptConnect();
    proDlg->close();
}
