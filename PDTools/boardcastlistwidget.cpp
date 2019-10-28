/*************************************************
    Copyright: 2018-2019 Melux
    Author: ChengHang
    Date:2019-01-03
    Description: 扫描列表
**************************************************/
#include "boardcastlistwidget.h"

/**
 * @brief 构造函数
 * @param parent
 */
BoardCastListWidget::BoardCastListWidget(QWidget *parent) : QWidget(parent)
{
    //初始化部分元素H
    detectDevices = new QStringList;
    scanChangeList = new QStringList;
    scanChangeList->append("扫描中");
    scanChangeList->append("扫描中.");
    scanChangeList->append("扫描中..");
    scanChangeList->append("扫描中...");

    //读取配置文件
    QFile cfg("config.ini");
    if(!cfg.exists())
    {
        QMessageBox::critical(NULL, "错误", "缺失配置文件！请检查目录下是否存在config.ini");
        return;
    }
    QSettings *config = new QSettings("config.ini", QSettings::IniFormat);
    boardcastAddress = config->value("Global/boardcastAddress").toString();
    configLocalIP = config->value("Global/localIP").toString();


    //获取本机IP与MAC
    QString localIP = Util::getHostIpAddress();

    QString localMAC = Util::getHostMacAddress();
    //UDP Socket 实例化
    broadcastPort = 18000;
    broadcastSocket = new QUdpSocket(this);

    bind_state = broadcastSocket->bind(QHostAddress(configLocalIP == "" ? localIP : configLocalIP));
    //bind_state = broadcastSocket->bind(broadcastPort, QUdpSocket::ShareAddress);
    broadcastSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);//禁止本机接收
    //计时器实例化
    broadcastTimer = new QTimer(this);
    scanChangeTimer = new QTimer(this);
    //信号槽
    connect(broadcastTimer, SIGNAL(timeout()), this, SLOT(broadcastDevices()));
    connect(scanChangeTimer, SIGNAL(timeout()), this, SLOT(scanChangeSlot()));
    connect(broadcastSocket, SIGNAL(readyRead()), this, SLOT(hasDevicesRespone()));
    //启用计时器
    broadcastTimer->start(1000);
    scanChangeTimer->start(200);
    //随机8位字节广播内容
    qsrand(QDateTime::currentMSecsSinceEpoch());

    const char chrs[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int chrs_size = sizeof(chrs);
    int randomx = 0;
    for (int i = 0; i < 8; ++i)
    {
       randomx= rand() % (chrs_size - 1);
       broadcastStr.insert(i, chrs[randomx]);
    }
}

/**
 * @brief 清理指针
 */
BoardCastListWidget::~BoardCastListWidget()
{
    broadcastTimer->stop();
    scanChangeTimer->stop();
}

/**
 * @brief 隐藏已连接的主机地址
 * @param currentIP 已连接的主机地址
 */
void BoardCastListWidget::hideConnected(QString currentIP)
{
    isHideConnected = true;
    connectedIP = currentIP;
}

/**
 * @brief 初始化
 * @param withCheckState 是否包含选择框
 */
void BoardCastListWidget::initListWidget(bool withCheckState)
{
    devicesList = new QListWidget(this);
    checkState = withCheckState;
    devicesList->setStyleSheet("border:none;background:none;padding:10px;font-size:14px;");
    devicesList->setGeometry(0, 0, this->width(), this->height());
    //信号槽
    connect(devicesList, &QListWidget::clicked, this, &BoardCastListWidget::devicesListClicked);
    connect(devicesList, &QListWidget::doubleClicked, this, &BoardCastListWidget::devicesListDoubleClicked);
    //扫描动画
    scanLabel = new QLabel(this);
    scanLabel->setText("扫描中");
    scanLabel->setAlignment(Qt::AlignCenter);
    scanLabel->setStyleSheet("font-size:16px;color:rgb(56, 99, 154);");
    scanLabel->setGeometry(0, (devicesList->height()-60)/2, devicesList->width(), 60);
}

/**
 * @brief 重新搜索设备
 */
void BoardCastListWidget::rescanDevices()
{
    devicesList->clear();
    detectDevices->clear();
    scanLabel->setHidden(false);
}

/**
 * @brief 获取扫描列表的选择列，或者选择所有列
 * @param getall 是否选择所有列
 * @return  QList<QString> 选中的列表
 */
QList<QString> BoardCastListWidget::getSelectRow(bool getall)
{
    QList<QString> selection;
    for(int i=0;i<detectDevices->size();i++)
    {
        QString item = QString("%1&%2").arg(detectDevices->at(i)).arg(i);
        if(getall)
        {
            //选择所有
            selection.append(item);
        }
        else if(devicesList->item(i)->checkState() == Qt::Checked)
        {
            //选择选中
            selection.append(item);
        }
    }
    return selection;
}

/**
 * @brief 显示指定row行的同步结果
 * @param row 行
 * @param state 状态值
 */
void BoardCastListWidget::setSyncResult(int row, QString state)
{

    if(state == "SUCCESS") //成功
    {
        devicesList->item(row)->setText(QString("%1 -> 同步成功").arg(detectDevices->at(row)));
        devicesList->item(row)->setTextColor(COLOR_SUCCESS);
    }
    if(state == "TIMEOUT") //超时
    {
        devicesList->item(row)->setText(QString("%1 -> 连接超时").arg(detectDevices->at(row)));
        devicesList->item(row)->setTextColor(COLOR_FAILED);
    }
    if(state == "SENDERR") //发送失败
    {
        devicesList->item(row)->setText(QString("%1 -> 发送失败").arg(detectDevices->at(row)));
        devicesList->item(row)->setTextColor(COLOR_FAILED);
    }
    if(state == "FAILED") //失败
    {
        devicesList->item(row)->setText(QString("%1 -> 同步失败").arg(detectDevices->at(row)));
        devicesList->item(row)->setTextColor(COLOR_FAILED);
    }

}

/**
 * @brief 把TCP报文结构体转为字节流以便发送
 * @param pkg 结构体
 * @return QByteArray 字节流
 */
QByteArray BoardCastListWidget::structToStream(pd_Package pkg)
{
    QByteArray sendBlock;
    QDataStream out (&sendBlock,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_8);
    out << (uchar)pkg.start <<(uchar)pkg.cmd
        << (uchar)pkg.len[0]<<(uchar)pkg.len[1]<<(uchar)pkg.len[2]<<(uchar)pkg.len[3];

    //计算长度
    int lengh;
    lengh = pkg.len[0];
    lengh = lengh << 8;
    lengh += pkg.len[1];
    lengh = lengh << 8;
    lengh += pkg.len[2];
    lengh = lengh << 8;
    lengh += pkg.len[3];
    out.writeRawData((char*)pkg.body, lengh);
    out << (uchar)pkg.check;
    //qDebug() << "sendBlock" << sendBlock;
    return sendBlock;
}

/**
 * @brief UDP广播局域网指定端口
 */
void BoardCastListWidget::broadcastDevices()
{
    //报文
    pd_Package_t pkg;
    char *body = broadcastStr.data();
    int lengh = (int)strlen(body);
    //校验
    unsigned char crc = Util::crc8((unsigned char *)body, lengh);

    pkg.start = START;
    pkg.cmd = COMMAND::SCAN;
    pkg.len[0] = lengh>>24;
    pkg.len[1] = lengh>>16;
    pkg.len[2] = lengh>>8;
    pkg.len[3] = lengh;
    pkg.body = (char*)body;
    pkg.check = crc;
    //字节流
    QByteArray message = structToStream(pkg);
    broadcastSocket->writeDatagram(message, message.size(),  boardcastAddress == "" ? QHostAddress::Broadcast : QHostAddress(boardcastAddress), broadcastPort);
}

/**
 * @brief 切换列表设定值来达到动态载入效果
 */
void BoardCastListWidget::scanChangeSlot()
{
    if(!bind_state)
    {
        scanLabel->setText("绑定UDP Socket失败\n广播无效");
        return;
    }
    QString currentText = scanLabel->text();
    int i = scanChangeList->indexOf(currentText);
    if(i == scanChangeList->size()-1)
    {
        //到达结尾重置
        scanLabel->setText(scanChangeList->at(0));
    }
    else
    {
        scanLabel->setText(scanChangeList->at(i+1));
    }
}

/**
 * @brief 广播后得到回应，检查状态，插入列表
 */
void BoardCastListWidget::hasDevicesRespone()
{
    //主机地址
    QHostAddress client_address;
    client_address.setAddress(QHostAddress::AnyIPv4);
    //读取缓冲区
    while(broadcastSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(broadcastSocket->pendingDatagramSize());
        broadcastSocket->readDatagram(datagram.data(), datagram.size(), &client_address);
        //检查头部
        if(datagram.size() < LEN_HEAD_PKG) return;
        if(datagram[0] != START) return;
        if(datagram[1] != (char)COMMAND::SCAN) return;
        //包长
        int pkg_len;
        unsigned char crc;
        pkg_len = datagram[2];
        pkg_len = pkg_len << 8;
        pkg_len += datagram[3];
        pkg_len = pkg_len << 8;
        pkg_len += datagram[4];
        pkg_len = pkg_len << 8;
        pkg_len += datagram[5];
        //包体
        QByteArray bodyJsonByte;
        for(int i=0;i<pkg_len;i++) bodyJsonByte.insert(i, datagram[i+6]);
        //检查校验
        crc = datagram[datagram.size()-1];
        if(!Util::checkCrc8Code((unsigned char *)bodyJsonByte.data(), pkg_len, crc)) return;

        //json parse
        QJsonParseError json_parse_error;
        QJsonDocument json_doucment = QJsonDocument::fromJson(bodyJsonByte, &json_parse_error);

        if(json_parse_error.error != QJsonParseError::NoError)
        {
            qDebug() << "BoardCast Respone Json Parse Error:" << Util::jsonParseError(json_parse_error.error);
            qDebug() << bodyJsonByte;
            return;
        }
        //json对象
        QJsonObject resIPObj = json_doucment.object();
        //获取值
        QString resIP = resIPObj["ip"].toString();
        QString resPort = resIPObj["port"].toVariant().toString();
        QString resDesc = resIPObj["desc"].toString();

        //隐藏已连接
        if(isHideConnected && resIP == connectedIP) continue;
        if(!resIP.isEmpty() && !resPort.isEmpty()) //数据有效
        {
            QString ipAndPort = QString("%1:%2").arg(resIP).arg(resPort);
            //新设备
            if(!detectDevices->contains(ipAndPort))
            {
                //插入到队列中
                QListWidgetItem *add_item = new QListWidgetItem(devicesList);
                detectDevices->append(ipAndPort);
                //devicesList->addItem(ipAndPort);
                add_item->setIcon(QIcon(":/pixmap/images/devices.png"));
                add_item->setText(ipAndPort);
                if(checkState)
                {
                    add_item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
                    add_item->setCheckState(Qt::Unchecked);
                }
                add_item->setToolTip(resDesc == "" ? "暂无说明" : resDesc);
                devicesList->addItem(add_item);
                scanLabel->setHidden(true);
            }

        }

    }

}

/**
 * @brief 扫描列表单击事件的槽函数
 * @param index 模型索引对象
 */
void BoardCastListWidget::devicesListClicked(const QModelIndex &index)
{
    //分割IP与端口
    QString t = detectDevices->at(index.row());
    QList<QString> ip = t.split(":");
    if(ip.size() >= 2)
    {
        //发送信号
        emit singleClick(ip[0], ip[1].toInt());
    }
    else
    {
        QMessageBox::warning(this, "警告", "IP地址不正确！");
        return;
    }
}

/**
 * @brief 扫描列表双击事件的槽函数
 * @param index 模型索引对象
 */
void BoardCastListWidget::devicesListDoubleClicked(const QModelIndex &index)
{
    //发送信号
    emit doubleClick();
}
