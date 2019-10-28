/*************************************************
   Copyright:HLD
   Author: HLD
   Date:2018-07-25
   Description:通用工具类(windows)
**************************************************/
#include "util.h"



/*
 * 系统信息参数
*/
static int processor_count_ = -1;
static int64_t last_time_ = 0;
static int64_t last_system_time_ = 0;

FILETIME now;
FILETIME creation_time;
FILETIME exit_time;
FILETIME kernel_time;
FILETIME user_time;
int64_t system_time;
int64_t time;
int64_t system_time_delta;
int64_t time_delta;
auto file_time_2_utc = [](const FILETIME* ftime) {
    LARGE_INTEGER li;
    li.LowPart = ftime->dwLowDateTime;
    li.HighPart = ftime->dwHighDateTime;
    return li.QuadPart;
};

int cpu = -1;

/**
 * @brief 设置文件隐藏
 * @param filePath
 * @return 操作状态
 */
bool Util::setHideAttribute(QString filePath)
{
    std::string str = filePath.toStdString();
    std::wstring wstr(str.length(), L' ');
    std::copy(str.begin(), str.end(), wstr.begin());
    LPCWSTR path = wstr.c_str();

    DWORD dwResult = ::GetFileAttributes( path );
    if( INVALID_FILE_ATTRIBUTES == dwResult )
    {
       return false;
    }
    if( !(FILE_ATTRIBUTE_HIDDEN & dwResult) ) // 如果它不是隐藏的
    {
       if( INVALID_FILE_ATTRIBUTES == ::SetFileAttributes( path, dwResult | FILE_ATTRIBUTE_HIDDEN ) )
       {
           return false;
       }
       return true;
    }
    else// 如果它已经是隐藏的，就当设置成功了
    {
       return true;
    }
}

/**
 * @brief  将数字版本转为字符串
 * @param num_ver int类型的版本号
 * @return 格式化的版本号
 */
QString Util::ver2str(int num_ver)
{
    if(num_ver == 0)
    {
        return " [Loading] ";
    }
    else
    {
        QString str_ver = QString::number(num_ver, 10);
        str_ver.insert(1, '.');
        str_ver.insert(3, '.');

        return str_ver;

    }
}

/**
 * @brief 从本地dat文件中读取版本号
 * @return int 数字版本号
 */
int Util::getVersionFromDAT()
{
    int ver = 0;
    //读取文件
    QFile updateFile("update.DAT");
    if(updateFile.open(QIODevice::ReadOnly))
    {
        //解析JOSN
        QJsonParseError json_parse_error;
        QJsonDocument json_doucment = QJsonDocument::fromJson(updateFile.readAll(), &json_parse_error);

        ver = json_doucment.object().value("latest").toInt();
    }
    updateFile.close();
    return ver;
}

/**
 * @brief QString转StdString
 * @param qstr QString字符串
 * @return string std string字符串
 */
std::string Util::QStr2StdStr(const QString qstr)
{
    QByteArray cdata = qstr.toLocal8Bit();
    return std::string(cdata);
}

/**
 * @brief 检测文件夹是否存在，不存在则创建
 * @param QString fullPath 路径
 * @return bool 是否存在，不存在则返回创建的状态
 */
bool Util::isDirExist(QString fullPath)
{
    QDir dir;
    if(dir.exists(fullPath))
    {
      return true;
    }
    else
    {
       bool ok = dir.mkdir(fullPath);//不存在则创建
       return ok;
    }
}

/**
 * @brief 删除文件夹及其内容
 * @param QString dirName 文件夹路径
 * @return bool 结果
 */
bool Util::deleteDir(const QString &dirName)
{
    QDir directory(dirName);
    if (!directory.exists())
    {
        return true;
    }


    QString srcPath = QDir::toNativeSeparators(dirName);
    if (!srcPath.endsWith(QDir::separator()))
        srcPath += QDir::separator();


    QStringList fileNames = directory.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    bool error = false;
    for (QStringList::size_type i=0; i != fileNames.size(); ++i)
    {
        QString filePath = srcPath + fileNames.at(i);
        QFileInfo fileInfo(filePath);
        if (fileInfo.isFile() || fileInfo.isSymLink())
        {
            QFile::setPermissions(filePath, QFile::WriteOwner);
            if (!QFile::remove(filePath))
            {
                qDebug() << "remove file" << filePath << " faild!";
                error = true;
            }
        }
        else if (fileInfo.isDir())
        {
            if (!deleteDir(filePath))
            {
                error = true;
            }
        }
    }


    if (!directory.rmdir(QDir::toNativeSeparators(directory.path())))
    {
        qDebug() << "remove dir" << directory.path() << " faild!";
        error = true;
    }
    return !error;
}

/**
 * @brief 获取当前进程资源使用率
 * @param SystemInfo &info 保存系统信息的结构体
 */
void Util::getSystemInfo(SystemInfo &info)
{
    //memory usage
    HANDLE currentProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(currentProcess, &pmc, sizeof(pmc));
    info.memoryUsage = (double)(pmc.PeakWorkingSetSize) / (1024 * 1024);

    //cpu usage
    if(processor_count_ == -1)
    {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        processor_count_ = (int)info.dwNumberOfProcessors;
    }

    GetSystemTimeAsFileTime(&now);

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId());
    if (!GetProcessTimes(hProcess, &creation_time, &exit_time, &kernel_time, &user_time))
    {
        return;
    }
    system_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time))
        / processor_count_;
    time = file_time_2_utc(&now);

    if ((last_system_time_ == 0) || (last_time_ == 0))
    {
        last_system_time_ = system_time;
        last_time_ = time;
        return;
    }
    system_time_delta = system_time - last_system_time_;
    time_delta = time - last_time_;

    if (time_delta == 0) return;
    info.cpuUsage = (system_time_delta * 100 + time_delta / 2) / time_delta;
    last_system_time_ = system_time;
    last_time_ = time;
}

/**
 * @brief 调用外部程序检测新版本(已弃用)
 * @param software
 * @param version
 * @param update_manual
 */
void Util::checkUpdate(QString software, int version, bool update_manual)
{
    terminateExe("updateClient.exe");
    QFile upclt_temp("updateClient.exe.temp");
    if(upclt_temp.exists())
    {
        QFile::remove("updateClient.exe");
        upclt_temp.rename("updateClient.exe");
    }

    if(!isNetWorkOnline())
    {
        qDebug() << "No network connection";
        return;
    }

    CheckUpdate *update = new CheckUpdate;
    update->getVersion(QString("http://192.168.0.230:8850/checkVersion/%1").arg(software), version, update_manual);
}

/**
 * @brief 调用外部程序检测新版本
 * @param software 更新程序名称
 */
void Util::checkUpdateByClient(QString software)
{
    QString currentexe = qApp->applicationFilePath().split("/").last();

    terminateExe("updateClient.exe");
    QFile upclt_temp("updateClient.exe.temp");
    if(upclt_temp.exists())
    {
        QFile::remove("updateClient.exe");
        upclt_temp.rename("updateClient.exe");
    }

    QString vc12upclt = "http://192.168.0.230:8850/static/softwares/system/vc12/updateClient.exe";
    QFile updateClient("updateClient.exe");
//    if(!updateClient.exists())
//    {
//        DownLoadFile *download = new DownLoadFile();
//        download->getFile(vc12upclt, true, false);
//        delete download;
//    }
    if(updateClient.exists())
    {
        QString client_path = qApp->applicationDirPath() + "/updateClient.exe";
        QProcess *updateTask = new QProcess;
        //QString request_url = QString("http://192.168.0.230:8850/checkLatest/%1").arg(software);
        bool p_state = updateTask->startDetached(client_path, QStringList() << currentexe);

        if(!p_state){
            QFile log("UpdateError.log");
            log.open(QIODevice::Append | QIODevice::Text);
            QTextStream out(&log);
            out << "================================" << endl;
            out << QDateTime::currentDateTime().toString() << endl;
            out << QString("错误说明: 启动更新器失败") << endl;
            out << QString("错误详情: %1").arg(updateTask->errorString()) << endl;
            out << QString("启动路径: %1").arg(client_path) << endl;
            out << QString("参数 1: %1").arg(currentexe) << endl;

            log.close();

            char *cmd;
            QString qCmd = QString("%1 %2 %3").arg(client_path).arg(currentexe);
            QByteArray cmdQA = qCmd.toUtf8();
            cmd = cmdQA.data();
            int res = system(cmd);
            if(res == 1){
                QFile log("UpdateError.log");
                log.open(QIODevice::Append | QIODevice::Text);
                QTextStream out(&log);
                out << "================================" << endl;
                out << QDateTime::currentDateTime().toString() << endl;
                out << QString("错误说明: 启动更新器失败") << endl;
                out << QString("错误详情: CMD模式") << endl;
                out << QString("启动路径: %1").arg(qCmd) << endl;
                log.close();
            }
        }
    }
    else
    {
        QMessageBox::critical(nullptr, "下载失败", "因缺少更新器<font color='red'>updateClient.exe</font>程序无法自动更新，请手动下载更新器(需高德办公室内网环境)<br><a href='"+vc12upclt+"'>"+vc12upclt+"</a><br>下载后置于程序目录并重新启动程序。");
        return;
    }

}

/**
 * @brief 检查网络状况
 * @return 无网返回false
 */
bool Util::isNetWorkOnline()
{
    QNetworkConfigurationManager mgr;
    return mgr.isOnline();
}

/**
 * @brief 调整图像自适应Label
 * @param qImage 调整的图像的引用
 * @param qLabel 适应Label的指针
 * @return QImage 调整后的图像
 */
QImage Util::ScaleImage2Label(const QImage &qImage, QLabel *qLabel)
{
    QSize qImageSize = qImage.size();
    QSize qLabelSize = qLabel->size();
    QImage qScaledImage(qLabelSize, QImage::Format_Indexed8);
    double dWidthRatio = 1.0 * qImageSize.width() / qLabelSize.width();
    double dHeightRatio = 1.0 * qImageSize.height() / qLabelSize.height();
    if (dWidthRatio>dHeightRatio)
    {
        qScaledImage = qImage.scaledToWidth(qLabelSize.width());
    }
    else
    {
        qScaledImage = qImage.scaledToHeight(qLabelSize.height());
    }
    return qScaledImage;
}

/**
 * @brief 强制关闭一个进程
 * @param const char *exe 待关闭的进程名
 */
void Util::terminateExe(const char *exe)
{
    HANDLE hSnapShot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

    //获得了所有进程的信息。
    //将从hSnapShot中抽取数据到一个PROCESSENTRY32结构中
    //这个结构代表了一个进程，是ToolHelp32 API的一部分。
    //抽取数据靠Process32First()和Process32Next()这两个函数。
    //这里我们仅用Process32Next()，他的原形是：
    //BOOL WINAPI Process32Next(HANDLE hSnapshot,LPPROCESSENTRY32 lppe);
    //我们程序的代码中加入：
    PROCESSENTRY32* processInfo = new PROCESSENTRY32;
    // 必须设置PROCESSENTRY32的dwSize成员的值 ;
    processInfo->dwSize = sizeof(PROCESSENTRY32);
    int index = 0;
    //这里我们将快照句柄和PROCESSENTRY32结构传给Process32Next()。
    //执行之后，PROCESSENTRY32 结构将获得进程的信息。我们循环遍历，直到函数返回FALSE。
    //printf("****************开始列举进程****************/n");
    int ID = 0;
    while(Process32Next(hSnapShot,processInfo) != FALSE)
    {
        index++;
        //printf("****************** %d ******************/n",index);
        //printf("PID       Name      Current Threads/n");
        //printf("%-15d%-25s%-4d/n",processInfo->th32ProcessID,processInfo->szExeFile,processInfo->cntThreads);
        int size = WideCharToMultiByte(CP_ACP, 0, processInfo->szExeFile, -1, NULL, 0, NULL, NULL);
        char *ch = new char[size+1];
        if(WideCharToMultiByte(CP_ACP, 0, processInfo->szExeFile, -1, ch, size, NULL, NULL))
        {
            if(strstr(ch, exe))//使用这段代码的时候只需要改变"cmd.exe".将其改成你要结束的进程名就可以了。
            {
                ID = processInfo->th32ProcessID;
                // qDebug()<<"ID ="<<ID;
                HANDLE hProcess;
                // 现在我们用函数 TerminateProcess()终止进程：
                // 这里我们用PROCESS_ALL_ACCESS
                hProcess=OpenProcess(PROCESS_ALL_ACCESS,TRUE,ID);
                //if(hProcess==NULL)
                //{
                  //  printf("Unable to get handle of process: ");
                  //  printf("Error is: %d",GetLastError());
                //}
                TerminateProcess(hProcess,0);
                CloseHandle(hProcess);
           }
        }
    }
    CloseHandle(hSnapShot);
    delete processInfo;
}

/**
 * @brief 获取软件信息
 */
void Util::getSoftwareInfomation()
{
    //构造窗口
    QDialog *infomationDialog = new QDialog();
    infomationDialog->setWindowTitle("软件信息");
    infomationDialog->setStyleSheet("font-size:14px;padding-left:20px");
    infomationDialog->setFixedSize(240, 180);
    infomationDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    QVBoxLayout *vbox = new QVBoxLayout(infomationDialog);
    vbox->addWidget(new QLabel(QString("软件名称: %1").arg(qApp->applicationDisplayName())));
    QString version_str = QString("当前版本: %1").arg(Util::ver2str(Util::getVersionFromDAT()));
    vbox->addWidget(new QLabel(version_str));
    vbox->addWidget(new QLabel("软件作者: ChengHang"));
    //VC12
    #if _MSC_VER == 1800
    vbox->addWidget(new QLabel("编译器: Microsoft VC++ 12"));
    //VC14
    #elif _MSC_VER == 1900
    vbox->addWidget(new QLabel("Compiler: Microsoft VC++ 14"));
    //MINGW
    #else
    vbox->addWidget(new QLabel("Compiler: GCC 5.3.0"));
    #endif
    vbox->addStretch(5);
    vbox->addWidget(new QLabel("邮箱: <a href='chenghang@melux.com'>chenghang@melux.com</a>"));
    vbox->addWidget(new QLabel("网站: <a href='www.melux.com'>www.melux.com</a>"));
    vbox->addStretch(10);
    QLabel *copyright = new QLabel("Copyright 2018-2020 Melux");
    copyright->setStyleSheet("font-size:12px;color:#666");
    vbox->addWidget(copyright);
    vbox->addStretch();
    infomationDialog->setLayout(vbox);
    infomationDialog->exec();
    delete infomationDialog;
}


/**
 * @brief 计算CRC8
 * @param ptr
 * @param len
 * @return crc8
 */
unsigned char Util::crc8(unsigned char *ptr, int len)
{
    unsigned char crc = 0x00;
    while (len--)
    {
        crc = crc_table[crc ^ *ptr++];
    }
    return (crc);
}

/**
 * @brief 校验CRC8
 * @param ptr
 * @param len
 * @param crc
 * @return bool
 */
bool Util::checkCrc8Code(unsigned char *ptr, int len,  unsigned char crc)
{
    return (uchar)crc == crc8(ptr, len);
}

/**
 * @brief 计算CRC16
 * @param data
 * @return crc16
 */
quint16 Util::crc16ForModbus(const QByteArray &data)
{
    static const quint16 crc16Table[] =
    {
        0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
        0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
        0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
        0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
        0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
        0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
        0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
        0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
        0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
        0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
        0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
        0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
        0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
        0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
        0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
        0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
        0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
        0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
        0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
        0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
        0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
        0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
        0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
        0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
        0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
        0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
        0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
        0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
        0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
        0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
        0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
        0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
    };

    quint8 buf;
    quint16 crc16 = 0xFFFF;

    for ( auto i = 0; i < data.size(); ++i )
    {
        buf = data.at( i ) ^ crc16;
        crc16 >>= 8;
        crc16 ^= crc16Table[ buf ];
    }

    return crc16;
}

/**
 * @brief 获取IP地址
 * @return ip地址
 */
QString Util::getHostIpAddress()
{
    QString strIpAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // 获取第一个本主机的IPv4地址

    int nListSize = ipAddressesList.size();
    for (int i = 0; i < nListSize; ++i)
    {
        if(ipAddressesList.at(i).toString().contains("192.168") || ipAddressesList.at(i).toString().contains("10.0")) {
            strIpAddress = ipAddressesList.at(i).toString();
            break;
        }
        else if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
           ipAddressesList.at(i).toIPv4Address()) {
           strIpAddress = ipAddressesList.at(i).toString();
        }
     }
     // 如果没有找到，则以本地IP地址为IP
     if (strIpAddress.isEmpty())
        strIpAddress = QHostAddress(QHostAddress::LocalHost).toString();
     //qDebug() << ipAddressesList;
     return strIpAddress;
}

/**
 * @brief 获取MAC地址
 * @return QString mac地址
 */
QString Util::getHostMacAddress()
{
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();// 获取所有网络接口列表
    int nCnt = nets.count();
    QString strMacAddr = "";
    for(int i = 0; i < nCnt; i ++)
    {
        // 如果此网络接口被激活并且正在运行并且不是回环地址，则就是我们需要找的Mac地址
        if(nets[i].flags().testFlag(QNetworkInterface::IsUp) && nets[i].flags().testFlag(QNetworkInterface::IsRunning) && !nets[i].flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            strMacAddr = nets[i].hardwareAddress();
            break;
        }
    }
    return strMacAddr;
}

/**
 * @brief JSON解析失败转中文
 * @param error 解析错误枚举
 * @return QString 中文错误
 */
QString Util::jsonParseError(QJsonParseError::ParseError error)
{
    QString error_str = "";
    switch (error) {
    case QJsonParseError::UnterminatedObject:
        error_str = "对象不正确地终止以右花括号结束";
        break;
    case QJsonParseError::MissingNameSeparator:
        error_str = "分隔不同项的逗号丢失";
        break;
    case QJsonParseError::UnterminatedArray:
        error_str = "数组不正确地终止以右中括号结束";
        break;
    case QJsonParseError::MissingValueSeparator:
        error_str = "对象中分割 key/value 的冒号丢失";
        break;
    case QJsonParseError::IllegalValue:
        error_str = "值是非法的";
        break;
    case QJsonParseError::TerminationByNumber:
        error_str = "在解析数字时，输入流结束";
        break;
    case QJsonParseError::IllegalNumber:
        error_str = "数字格式不正确";
        break;
    case QJsonParseError::IllegalEscapeSequence:
        error_str = "在输入时，发生一个非法转义序列";
        break;
    case QJsonParseError::IllegalUTF8String:
        error_str = "在输入时，发生一个非法 UTF8 序列";
        break;
    case QJsonParseError::UnterminatedString:
        error_str = "字符串不是以引号结束";
        break;
    case QJsonParseError::MissingObject:
        error_str = "没有找到一个预期应该存在的对象";
        break;
    case QJsonParseError::DeepNesting:
        error_str = "对解析器来说，JSON 文档嵌套太深";
        break;
    case QJsonParseError::DocumentTooLarge:
        error_str = "对解析器来说，JSON 文档太大";
        break;
    case QJsonParseError::GarbageAtEnd:
        error_str = "解析的文档在末尾处包含额外的乱码";
        break;
    default:
        break;
    }
    return error_str;
}

CheckUpdate::CheckUpdate()
{
    currentexe = qApp->applicationFilePath().split("/").last();

}


void CheckUpdate::getVersion(const QString &url, const int version, bool update_manual)
{
    request_url = url;
    current_version = version;
    manual = update_manual;
    checkThread = new QThread;
    ck = new ThreadCheck(url, version);
    ck->form = this;
    ck->moveToThread(checkThread);
    checkThread->start();
    connect(checkThread, &QThread::started, ck, &ThreadCheck::checkVersion);

}

void CheckUpdate::execUpdatePanel(const QString &changelog)
{
    int w = 300;
    int h = 300;
    QFont font("Microsoft YaHei", 14, 55);
    this->setGeometry((QApplication::desktop()->width()-w)/2, (QApplication::desktop()->height()-h)/2, w, h);
    this->setWindowTitle("版本更新");
    QVBoxLayout *vbox = new QVBoxLayout(this);
    QLabel *label_currentver = new QLabel(this);
    QLabel *label_latestver = new QLabel(this);
    QLabel *label_log = new QLabel("更新日志：", this);
    label_currentver->setText("当前版本：" + Util::ver2str(current_version));
    label_currentver->setFont(font);
    label_latestver->setText("最新版本：" + Util::ver2str(latest_version));
    label_latestver->setFont(font);
    label_latestver->setStyleSheet("color: #F10000");
    QTextBrowser *updatelog = new QTextBrowser();
    updatelog->setText(changelog);
    QPushButton *btn = new QPushButton("下载更新", this);

    btn->setStyleSheet("padding:10px");
    QObject::connect(btn, SIGNAL(clicked(bool)), this, SLOT(startUpdate()));
    vbox->addWidget(label_currentver);
    vbox->addWidget(label_latestver);
    vbox->addSpacing(10);
    vbox->addWidget(label_log);
    vbox->addSpacing(5);
    vbox->addWidget(updatelog);
    vbox->addWidget(btn);
    this->setLayout(vbox);
    this->exec();
}

void CheckUpdate::generateUpdateScript()
{
    #if defined(Q_OS_WIN32)
        QFile updateFile("update.HLD");
        if(updateFile.open(QIODevice::WriteOnly))
        {
            QTextStream out(&updateFile);
            for(QString f : filenames)
            {
                out << f << endl;
            }
            updateFile.close();
            //
            QString vc12upclt = "http://192.168.0.230:8850/static/softwares/system/vc12/updateClient.exe";
            QFile updateClient("updateClient.exe");
//            if(!updateClient.exists())
//            {
//                DownLoadFile *download = new DownLoadFile();
//                download->getFile(vc12upclt, true, false);
//                delete download;
//            }
            if(updateClient.exists())
            {
                QProcess updateTask;
                updateTask.startDetached("updateClient.exe", QStringList() << currentexe);
                qApp->exit(0);
            }
            else
            {
                QMessageBox::critical(nullptr, "下载失败", "因缺少更新器<font color='red'>updateClient.exe</font>程序无法自动更新，请手动下载更新器(高德办公室内网环境下)<br><a href='"+vc12upclt+"'>"+vc12upclt+"</a><br>下载后置于程序目录并手动运行updateClient.exe以更新程序。");
                return;
            }
        }
        else
        {
            QMessageBox::critical(NULL, "错误", "写入更新脚本失败！请重试");
            return;
        }

    #elif defined(Q_OS_LINUX)
        QString filename = currentexe.split("/").last();
        QString filepath = QCoreApplication::applicationDirPath() + "/";
        QString scriptname = QCoreApplication::applicationDirPath().append("/update.sh");
        QFile file(scriptname);
        if(!file.open(QIODevice::ReadWrite))
        {
            QString error_msg = "写入更新脚本失败！请手动下载以下更新并替换同名文件";
            for(auto link : downloadfiles)
            {
                error_msg += "<br><a href="+link.toString()+">"+link.toString()+"</a>";
            }
            QMessageBox::critical(NULL, "错误", error_msg);
            return;
        }
        QTextStream stream(&file);
        stream << "#!/bin/sh\n";
        stream << "#\n";
        stream << "echo Update Script " << latest_version << "\n";
        stream << "killall -9 "+filename+"\n";
        for(QString f : filenames)
        {
            if(f.indexOf('$')>0){
                stream << "mkdir " << filepath << f.split('$')[0] << "\n";
                stream << "mv " << filepath << f << ".temp " << filepath << f.split('$')[0] << "\\" << f.split('$')[1] << "\n";
            }else{
                stream << "mv " << filepath << f << ".temp " << filepath << f << "\n";
            }
        }
        stream << "echo Ready Start" << "\n";
        stream << "chmod 777 " << QCoreApplication::applicationDirPath() <<"/" << filename << "\n";
        stream << QCoreApplication::applicationDirPath() <<"/" << filename << "\n";
        file.close();
        char *cmd;
        QByteArray ba = QString("sh %1").arg(scriptname).toLatin1();
        cmd=ba.data();
        qDebug() << cmd;
        system(cmd);
    #endif


}

CheckUpdate::~CheckUpdate()
{
    checkThread->quit();
    checkThread->wait();
    checkThread->deleteLater();
    delete ck;
}

void CheckUpdate::startUpdate()
{
    QProgressDialog progress("开始","取消", 0, downloadfiles.size(),  this);
    progress.setWindowTitle(tr("Converting..."));
    progress.setWindowModified(Qt::WindowModal);
    progress.show();

    for(int i=0;i<downloadfiles.size();i++)
    {
        QString link = downloadfiles[i].toString();
        QString filename = link.split("/").last();
        filenames << filename;
        progress.setLabelText(QString("正在下载 %1 ...").arg(filename));

        DownLoadFile *download = new DownLoadFile();
        download->getFile(link, true);
        delete download;
        progress.setValue(i);
        QCoreApplication::processEvents();
    }
    progress.setValue(downloadfiles.size());
    generateUpdateScript();
    this->close();
}

void CheckUpdate::replyCheckVersion(QNetworkReply *reply)
{
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject versionJson = doc.object();
    if(versionJson.value("status").toInt() == 200)
    {
        downloadfiles = versionJson.value("files").toArray();
        latest_version = versionJson.value("latest").toInt();
        if(latest_version == 0)
        {
            QMessageBox::warning(NULL, "警告", "该程序已经停止维护！请咨询开发人员获取详情.");
            return;
        }
        if(latest_version > current_version)
            execUpdatePanel(versionJson.value("changelog").toString());
        else
        {
            qDebug() << "Latest Version" << latest_version;
            if(manual) QMessageBox::information(NULL, "检查更新", "当前已是最新版本");
        }

    }else if(versionJson.value("status").toInt() == 301){
        //QString infomation = "该程序现在已经被一个更高的版本所替代，当前更新功能已不可用<br>请点击下载最新程序 -> <a href='"+dlink+"'>下载地址</a><br>链接：<a href='"+dlink+"'>"+dlink+"</a>";
        QString infomation = versionJson.value("infomation").toString();
        QMessageBox::warning(NULL, "警告", infomation);
        return;
    }else{
        QFile log("UpdateError.log");
        log.open(QIODevice::Append | QIODevice::Text);
        QTextStream out(&log);
        out << "================================" << endl;
        out << QDateTime::currentDateTime().toString() << endl;
        out << QString("错误说明：不正确的请求返回代码")  << endl;
        out << "请求地址：" + request_url << endl;
        out << "响应代码：" + versionJson.value("status").toString() << endl;
        out << "响应数据：" + doc.toJson() << endl;
        log.close();
        qWarning("Error CheckVersion, code: %d", versionJson.value("status").toInt());
    }
}

void CheckUpdate::loadError(QNetworkReply::NetworkError error)
{
    qDebug() << "Error Network Request";
    QFile log("UpdateError.log");
    log.open(QIODevice::Append | QIODevice::Text);
    QTextStream out(&log);
    out << "================================" << endl;
    out << QDateTime::currentDateTime().toString() << endl;
    out << QString("错误说明: 网络请求失败") << endl;
    out << QString("错误代码：%1").arg(error) << endl;
    log.close();
}

ThreadCheck::ThreadCheck(const QString &checkUrl, int ver)
{
    url = checkUrl;
    current_version = ver;
}


void ThreadCheck::checkVersion()
{
    #if defined(Q_OS_WIN32)
    platform = "win";
    #elif defined(Q_OS_LINUX)
    platform = "linux";
    QFile file(QCoreApplication::applicationDirPath().append("/update.sh"));
    if (file.exists())
    {
        if(file.open(QIODevice::ReadWrite))
        {
            file.readLine();
            file.readLine();
            int script_version = file.readLine().mid(19).replace("\n", "").toInt();
            qDebug() << "CHECKVERSION" << script_version;
            if(script_version != current_version)
            {
                QMessageBox::warning(NULL, "错误", "调用更新脚本失败，请关闭程序，运行目录下的<b><font color='red'>update.sh</font></b>更新文件。重复出现此问题请删除update.bat并重新下载更新，如继续出现，请联系软件开发人员。");
                return;
            }
            file.remove();
        }

    }

    #endif

    QNetworkRequest request;
    request.setUrl(QUrl(url + "/" + platform));

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QNetworkReply *reply = manager->get(request);
    connect(manager, &QNetworkAccessManager::finished, form, &CheckUpdate::replyCheckVersion);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), form, SLOT(loadError(QNetworkReply::NetworkError)));
}


DownLoadFile::DownLoadFile() : QObject()
{

}

void DownLoadFile::getFile(const QString &download_url, bool sync, bool tempname)
{
    QUrl url(download_url);
    if(tempname){
        file = new QFile(QApplication::applicationDirPath() + "/" + download_url.split("/").last() + ".temp");
    }else{
        file = new QFile(QApplication::applicationDirPath() + "/" + download_url.split("/").last());
    }
    file->open(QIODevice::WriteOnly);

    QNetworkAccessManager *accessManager = new QNetworkAccessManager(this);
    accessManager->setNetworkAccessible(QNetworkAccessManager::Accessible);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    reply = accessManager->get(request);

    connect((QObject *)reply, SIGNAL(readyRead()), this, SLOT(readContent()));
    //connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(loadError(QNetworkReply::NetworkError)));
    connect(accessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    if(sync)
    {
        QEventLoop eventLoop;
        connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
        eventLoop.exec();
    }


//    connect(reply, SIGNAL(downloadProgress(qint64 ,qint64)), this, SLOT(loadProgress(qint64 ,qint64)));


}

bool DownLoadFile::isFinished()
{
    return download_status;
}

void DownLoadFile::readContent()
{
    file->write(reply->readAll());
}

void DownLoadFile::replyFinished(QNetworkReply *)
{
    if(reply->error() == QNetworkReply::NoError)
    {
        reply->deleteLater();
        file->flush();
        file->close();
        download_status = true;
    }
    else
    {
        file->remove();
        QMessageBox::critical(NULL, "下载失败", "Failed!!!");
        return;
    }
}

void DownLoadFile::loadError(QNetworkReply::NetworkError)
{
    QMessageBox::critical(NULL, "下载失败", QString("错误代码: %1").arg(reply->error()));
}
