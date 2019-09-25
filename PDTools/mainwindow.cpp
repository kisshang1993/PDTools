/*************************************************
    Copyright: 2018-2019 Melux
    Author: ChengHang
    Date:2018-05-14
    Description: 主窗口
**************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //初始化
    ui->setupUi(this);
    //自定义标题栏
    //TitleBar *titleBar = new TitleBar(this);
    qsrand(QDateTime::currentMSecsSinceEpoch());
    QFile qss(":/skin/white.qss");
    qss.open(QFile::ReadOnly);
    qApp->setStyleSheet(qss.readAll());
    qss.close();
    uploadFirmwareProgressBar = new QProgressBar;
    //设置配置修改器参数
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setSortingEnabled(false);
    ui->tableView->verticalHeader()->hide();
    ui->tableView->setWordWrap(false);
    ui->tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableView->setShowGrid(false);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->horizontalHeader()->setHighlightSections(false);
    ui->tableView->setAlternatingRowColors(true);  // alternative colors
    ui->tableView->setFrameShape(QFrame::NoFrame);
    ui->jsonSelectComboBox->setView(new QListView());
    statusBar()->setStyleSheet("QStatusBar::item{border: 0px}");
    statusBar()->setGeometry(-5, -5, statusBar()->width()-10, statusBar()->height());

    //Tree model 搜索框样式
    ui->searchTreelineEdit->setTextMargins(0, 0, 30, 0);
    QAction *searchAction = new QAction(ui->searchTreelineEdit);
    searchAction->setIcon(QIcon(":/pixmap/images/search.png"));
    ui->searchTreelineEdit->addAction(searchAction, QLineEdit::TrailingPosition);

    //等待接受数据
    waitLabel = new QLabel(this);
    waitLabel->setFont(QFont("Microsoft YaHei", 20, 33));
    waitLabel->setText("等待接受数据");
    waitLabelText = new QStringList;
    waitLabelText->append("等待接受数据");
    waitLabelText->append("等待接受数据.");
    waitLabelText->append("等待接受数据..");
    waitLabelText->append("等待接受数据...");
    waitLabelTimer = new QTimer(this);
    connect(waitLabelTimer, &QTimer::timeout, this, &MainWindow::waitLabelTextChange);
    waitLabelTimer->start(200);
    waitLabel->setGeometry((width()-250)/2, (height()-80)/2, 250, 80);
    waitLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    waitLabel->setStyleSheet("padding-left: 20px;border: 2px solid rgb(111, 156, 207);background: #fdfdfd;border-radius:5px");
    waitLabel->show();
    //载入内存使用信息
    memoryUsageStatus = new QLabel("资源信息载入中...");
    memoryUsageStatus->setStyleSheet("color: #004EA1");

}

//void MainWindow::paintEvent(QPaintEvent *)
//{
//    //绘制阴影
//    QPainter painter(this);
//    QColor color(0, 0, 0, 0);
//    int shadowWidth = 5;
//    for(int i=0; i<shadowWidth; i++)
//    {
//        QPainterPath path;
//        path.setFillRule(Qt::WindingFill);
//        path.addRect(shadowWidth-i, shadowWidth-i, this->width()-(shadowWidth-i)*2, this->height()-(shadowWidth-i)*2);
//        color.setAlpha(30 - i*5);
//        painter.setPen(color);
//        painter.drawPath(path);
//    }
//    painter.fillRect(shadowWidth, shadowWidth, width()-shadowWidth*2,height()-shadowWidth*2, QColor(255, 255, 255));

//}

/**
 * @brief 查找指定键的所有父级
 * @param obj 查找的JSON对象
 * @param key 键
 * @return 父级列表
 * @note 返回列表格式为：p0 << p1 << k
 */
QStringList MainWindow::findKey(const QJsonObject &obj, QString &key)
{
    QStringList list;
    for(QJsonObject::const_iterator it1=obj.constBegin();it1!=obj.constEnd();it1++)
    {
        //qDebug() << it1.key();
        if(it1.key() == key){
            list << it1.key();
            return list;
        }

        if(it1.value().isObject())
        {
            QJsonObject jsonc1_obj = it1.value().toObject();
            for(QJsonObject::Iterator it2=jsonc1_obj.begin();it2!=jsonc1_obj.end();it2++)
            {
                //qDebug() << " " << it2.key();
                if(it2.key()==key){
                    list << it1.key() << it2.key();
                    return list;
                }
                QJsonObject jsonc2_obj = it2.value().toObject();
                for(QJsonObject::Iterator it3=jsonc2_obj.begin();it3!=jsonc2_obj.end();it3++)
                {
                    //qDebug() << "    " << it3.key();
                    if(it3.key()==key){
                        list << it1.key() << it2.key() << it3.key();
                        return list;
                    }
                }
            }
        }
    }

    return list;
}

/**
 * @brief 查找指定键的注释，没有则返回其本身
 * @param key 键
 * @return QString 注释
 */
QString MainWindow::findDescForKey(QString &key)
{

    for(QJsonObject::const_iterator it1=jsonUiObj.constBegin();it1!=jsonUiObj.constEnd();it1++)
    {

        if(it1.key() == key)
        {
            return it1.value().toObject()["desc"].toString();
        }

        if(it1.value().isObject())
        {
            QJsonObject jsonc1_obj = it1.value().toObject();
            for(QJsonObject::Iterator it2=jsonc1_obj.begin();it2!=jsonc1_obj.end();it2++)
            {
                if(it2.key()==key){
                    return it2.value().toObject()["desc"].toString();
                }
                QJsonObject jsonc2_obj = it2.value().toObject();
                for(QJsonObject::Iterator it3=jsonc2_obj.begin();it3!=jsonc2_obj.end();it3++)
                {
                    if(it3.key()==key){
                        return it3.value().toObject()["desc"].toString();
                    }
                }
            }
        }
    }

    return key;
}

/**
 * @brief 查找指定键的注释，没有则返回其本身
 * @param QStringList &keylist 修改的键
 * @param QString val 修改的值
 * @return bool 操作状态
 */
bool MainWindow::modifyVal(const QStringList &keyList, QString val)
{
    qDebug() << "modifyVal" << keyList << val;
    QVariant variant(val);
    //一级
    if(keyList.size()==1)
    {
        QJsonObject targetObj = jsonUiObj.take(keyList[0]).toObject();
        if(targetObj["value"].isDouble())
            targetObj["value"] = variant.toDouble();
        else if(targetObj["value"].isBool())
            targetObj["value"] = variant.toBool();
        else
            targetObj["value"] = variant.toString();
        jsonUiObj.insert(keyList[0], targetObj);
    }
    //二级
    else if(keyList.size()==2)
    {
        QJsonObject r1 = jsonUiObj.take(keyList[0]).toObject();
        QJsonObject targetObj = r1.take(keyList[1]).toObject();

        if(targetObj["value"].isDouble())
            targetObj["value"] = variant.toDouble();
        else if(targetObj["value"].isBool())
            targetObj["value"] = variant.toBool();
        else
            targetObj["value"] = variant.toString();
        r1.insert(keyList[1], targetObj);
        jsonUiObj.insert(keyList[0], r1);
    }
    //三级
    else if(keyList.size()==3)
    {
        QJsonObject r1 = jsonUiObj.take(keyList[0]).toObject();
        QJsonObject r2 = r1.take(keyList[1]).toObject();
        QJsonObject targetObj = r2.take(keyList[2]).toObject();

        if(targetObj["value"].isDouble())
            targetObj["value"] = variant.toDouble();
        else if(targetObj["value"].isBool())
            targetObj["value"] = variant.toBool();
        else
            targetObj["value"] = variant.toString();

        r2.insert(keyList[2], targetObj);
        r1.insert(keyList[1], r2);
        jsonUiObj.insert(keyList[0], r1);

    }else return false;

    //检查是某单改
    if(onlySave) return true;
    singleModify ? transmissionModify(keyList, val) : transmissionJSON();

    return true;
}

/**
 * @brief 清空模型并初始化数据
 */
void MainWindow::modelClear()
{
    model->clear();
    model->setHorizontalHeaderLabels(QStringList()<<"参数"<<"值");
    ui->tableView->setColumnWidth(0, 240);
    ui->tableView->setColumnWidth(1, 238);;
}

/**
 * @brief 主窗口析构函数
 */
MainWindow::~MainWindow()
{
    delete uploadFirmwareProgressBar;
    delete ui;
}

/**
 * @brief 结构树点击的槽函数
 * @param index 模型索引对象
 */
void MainWindow::treeClicked(const QModelIndex &index)
{
    QString k = index.data().toString();
    QStringList treelist = findKey(jsonUiObj, k);

    QJsonObject obj;
    //判断级别，取对应对象
    if(treelist.size() == 1)
    {
        obj = jsonUiObj;
    }

    if(treelist.size() >= 2 &&  jsonUiObj[treelist[0]].isObject())
    {
        obj = jsonUiObj[treelist[0]].toObject();
        k = treelist[1];
    }

    modelClear();

    int i = 0;
    for(QJsonObject::Iterator it1 = obj.begin();it1 != obj.end();it1++)
    {
        if(k == it1.key() && it1.value().isObject())
        {
            QJsonObject c1 = it1.value().toObject();
            if(c1.contains("value"))//是一个值对象
            {
                QStandardItem *item = new QStandardItem(QString("%1").arg(it1.key()));
                item->setToolTip(findDescForKey(it1.key()));
                model->setItem(i,0, item);
                model->setItem(i,1, new QStandardItem(QString("%1").arg(c1["value"].toVariant().toString())));
                i++;
            }
            else
            {
                for(QJsonObject::Iterator it2 = c1.begin();it2 != c1.end();it2++)
                {
                    QJsonObject c2 = it2.value().toObject();
                    if(c2.contains("value"))//是一个值对象
                    {
                        QStandardItem *item = new QStandardItem(QString("%1").arg(it2.key()));
                        item->setToolTip(findDescForKey(it2.key()));
                        model->setItem(i,0, item);
                        model->setItem(i,1, new QStandardItem(QString("%1").arg(c2["value"].toVariant().toString())));
                        i++;
                    }
                    else
                    {
                        for(QJsonObject::Iterator it3 = c2.begin();it3 != c2.end();it3++)
                        {
                            QJsonObject c3 = it3.value().toObject();
                            if(c3.contains("value"))//是一个值对象
                            {
                                QStandardItem *item = new QStandardItem(QString("%1").arg(it3.key()));
                                item->setToolTip(findDescForKey(it3.key()));
                                model->setItem(i,0, item);
                                model->setItem(i,1, new QStandardItem(QString("%1").arg(c3["value"].toVariant().toString())));
                                i++;
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief 配置编辑器内容击的槽函数,构建一个改值面板
 * @param index
 */
void MainWindow::tableClicked(const QModelIndex &index)
{
    //初始化、默认类别
    QString type = "string";
    QString k = model->index(index.row(), 0).data().toString();
    QVariant val = model->index(index.row(), 1).data();
    qDebug() << val;
    QStringList treelistUi = findKey(jsonUiObj, k);
    //json对象
    QJsonObject objUi;
    if(treelistUi.size()>=1)
        objUi = jsonUiObj[treelistUi[0]].toObject();

    if(treelistUi.size()>=2)
        objUi = objUi[treelistUi[1]].toObject();

    if(treelistUi.size()>=3)
        objUi = objUi[treelistUi[2]].toObject();

    //构造修改面板   
    EventQWidget *editWidget = new EventQWidget;
    editWidget->setWindowModality(Qt::ApplicationModal);
    editWidget->setWindowTitle("编辑值");
    editWidget->setFixedSize(300,240);
    editWidget->setWindowFlags(editWidget->windowFlags()&~Qt::WindowMaximizeButtonHint&~Qt::WindowMinimizeButtonHint);
    editWidget->move(x()+(width()-editWidget->width())/2, y()+(height()-editWidget->height())/2);
    //垂直布局
    QVBoxLayout *vbox = new QVBoxLayout(editWidget);
    QPushButton *editbtn = new QPushButton(onlySave ? "保存" : "写入", editWidget);

    QLabel *name = new QLabel(editWidget);
    name->setText(treelistUi.last());
    name->setStyleSheet("font-size: 15px;font-family:'microsoft yahei'");
    //数字输入框
    QLineEdit *stringInput = new QLineEdit(editWidget);
    //浮点数输入框
    QDoubleSpinBox *doubleInput = new QDoubleSpinBox(editWidget);
    //下拉选择框
    QComboBox *comboBox;
    doubleInput->setHidden(true);
    stringInput->setText(val.toString());
    QButtonGroup *boolgroup = new QButtonGroup(editWidget);
    //滑块
    QSlider *slider;
    QLabel *sliderValue;//当前值
    vbox->addWidget(name);
    vbox->addSpacing(3);
    vbox->addWidget(stringInput);

    //构造UI
    if(objUi.contains("type"))
    {
        type = objUi["type"].toString();
        //文本输入框
        if(type == "string")
        {
            if(objUi.contains("max"))
            {
                stringInput->setMaxLength(objUi["max"].toInt());
            }
        }
        //数字输入框
        if(type == "int")
        {
            int min, max;
            objUi.contains("min") ? min = objUi["min"].toInt() : min = 0;
            objUi.contains("max") ? max = objUi["max"].toInt() : max = 9999;
            stringInput->setValidator(new QIntValidator(min, max, editWidget));
        }
        //浮点数输入框
        if(type == "float" || type == "double")
        {
            int decimals;
            double min, max;

            objUi.contains("min") ? min = objUi["min"].toDouble() : min = 0.0;
            objUi.contains("max") ? max = objUi["max"].toDouble() : max = 9999.9;
            objUi.contains("decimals") ? decimals = objUi["decimals"].toInt() : decimals = 1;
            doubleInput->setRange(min, max);
            doubleInput->setDecimals(decimals);
            doubleInput->setValue(val.toDouble());
            stringInput->setHidden(true);
            doubleInput->setHidden(false);
            vbox->addWidget(doubleInput);
        }
        //单选框
        if(type == "bool")
        {
            QRadioButton *rbTrue = new QRadioButton(editWidget);
            rbTrue->setText("True");
            QRadioButton *rbFalse = new QRadioButton(editWidget);
            rbFalse->setText("False");
            boolgroup->addButton(rbTrue, 1);
            boolgroup->addButton(rbFalse, 0);
            QHBoxLayout *bghbox = new QHBoxLayout(editWidget);
            bghbox->addWidget(rbTrue);
            bghbox->addWidget(rbFalse);
            bghbox->addStretch();
            stringInput->setHidden(true);
            val.toBool() ? rbTrue->setChecked(true) : rbFalse->setChecked(true);
            vbox->addLayout(bghbox);
        }
        //列表选择
        if(type == "list") {
            stringInput->setHidden(true);
            comboBox = new QComboBox(editWidget);
            comboBox->setView(new QListView(editWidget));
            QJsonArray strList = objUi["list"].toArray();
            int setIndex = 0;
            for(int i=0;i<strList.size();i++)
            {
                QString s = strList[i].toString();
                if(s == val) setIndex = i;
                comboBox->addItem(s);
            }
            comboBox->setCurrentIndex(setIndex);
            vbox->addWidget(comboBox);

        }
        //滑块选择
        if(type == "slider") {
            stringInput->setHidden(true);
            slider = new QSlider(editWidget);
            slider->setOrientation(Qt::Horizontal);
            slider->setTickPosition(QSlider::TicksAbove);
            slider->setMinimum(objUi.contains("min") ? objUi["min"].toInt() : 0);
            slider->setMaximum(objUi.contains("max") ? objUi["max"].toInt() : 9999);
            slider->setSingleStep(objUi.contains("step") ? objUi["step"].toInt() : 10);
            slider->setValue(val.toInt());
            vbox->addWidget(slider);

            sliderValue = new QLabel(editWidget);
            sliderValue->setText(QString("Current Value: %1").arg(val.toString()));
            sliderValue->setStyleSheet("color: #000;font-size: 15px;font-family:'microsoft yahei'");
            vbox->addWidget(sliderValue);
            connect(slider, &QSlider::valueChanged, [=](){
                sliderValue->setText(QString("Current Value: %1").arg(slider->value()));
            });
            if(objUi["realtime"].toBool() == true)
            {
                editbtn->setDisabled(true);
                editbtn->setText("已开启实时写入");
                connect(slider, &QSlider::sliderReleased, [=](){
                    QString sliderValue = QString::number(slider->value());
                    model->setData(model->index(index.row(), 1), sliderValue);
                    modifyVal(treelistUi, sliderValue);
                });
            }
        }
    }
    //正则表达式
    if(objUi.contains("regexp"))
    {
        QRegExp rx(objUi["regexp"].toString());
        QRegExpValidator *reg = new QRegExpValidator(rx, editWidget);
        stringInput->setValidator(reg);
    }
    //只读
    if(objUi.contains("readonly") && objUi["readonly"].toString()=="true")
    {
        stringInput->setDisabled(true);
        doubleInput->setDisabled(true);
        comboBox->setDisabled(true);
        slider->setDisabled(true);
        editbtn->setDisabled(true);

    }
    vbox->addSpacing(5);
    //提示信息
    for(auto it=objUi.constBegin();it!=objUi.constEnd();it++)
    {
        QLabel *limit = new QLabel(editWidget);
        if(it.key() == "desc" || it.key() == "value" ||
           it.key() == "readonly" || it.key() == "type" ||
           it.key() == "realtime") continue;
        if(it.key() == "list")
        {
            limit->setText(QString("count: %1").arg(it.value().toArray().size()));
        }
        else
        {
            limit->setText(QString("%1: %2").arg(it.key()).arg(it.value().toVariant().toString()));
        }
        limit->setStyleSheet("font-size: 14px;font-family:'microsoft yahei';color:#484848;padding-left:5px");
        vbox->addWidget(limit);
    }

    vbox->addStretch();
    vbox->addWidget(editbtn);
    editWidget->setLayout(vbox);
    editWidget->show();

    //写入 匿名槽函数
    connect(editbtn,  &QPushButton::clicked, [=](){

        if(type == "string" || type == "int"){ //输入框
            model->setData(model->index(index.row(), 1), stringInput->text());
            modifyVal(treelistUi, stringInput->text());

        }else if(type == "float" || type == "double"){ //浮点数输入框
            model->setData(model->index(index.row(), 1), doubleInput->text());
            modifyVal(treelistUi, doubleInput->text());

        }else if(type == "bool"){ //单选框
            if(boolgroup->checkedId()==1){
                model->setData(model->index(index.row(), 1), "true");
                modifyVal(treelistUi, "true");
            }else{
                model->setData(model->index(index.row(), 1), "false");
                modifyVal(treelistUi, "false");
            }
        }
        else if(type == "list") { //下拉列表
            QString selectVal = comboBox->currentText();
            model->setData(model->index(index.row(), 1), selectVal);
            modifyVal(treelistUi, selectVal);
        }
        else if(type == "slider") { //滑块
            QString sliderValue = QString::number(slider->value());
            model->setData(model->index(index.row(), 1), sliderValue);
            modifyVal(treelistUi, sliderValue);
        }
        editWidget->close();
    });
    //ESC关闭
    connect(editWidget, &EventQWidget::pressEsc, [=](){
        editWidget->close();
    });
    //释放内存
    connect(editWidget, &EventQWidget::close, [=](){
        delete editWidget;
    });
}

/**
 * @brief 响应窗口关闭事件，协同关闭窗口
 * @param event
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    //关闭未关闭的对象
    if(palmImagePanel != nullptr) palmImagePanel->close();
    if(faceImagePanel != nullptr) faceImagePanel->close();
    //if(syncWidget != nullptr && syncWidget->inherits("QWidget")) syncWidget->close();
    isSocketManualClose = true;
    //断开Socket
    if(socket->isOpen())
    {
        socket->disconnectFromHost();
        socket->close();
    }
    this->close();
}

/**
 * @brief 引用全局Socket，以及取得版本号
 * @param mainSocket
 * @param ver
 */
void MainWindow::setSocket(QTcpSocket *mainSocket, int ver)
{
    socket = mainSocket;
    version = ver;
    //连接槽函数
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::socketReadData);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::socketDisconnect);
    //初始化请求
    sendRequest(COMMAND::INIT);

    initProDlg = new QProgressDialog;
    initProDlg->setRange(0, 0);
    initProDlg->setWindowTitle("PDTools Init");
    initProDlg->setLabelText("正在初始化程序...");
    initProDlg->show();
    connect(initProDlg, &QProgressDialog::canceled, this, &MainWindow::socketDisconnect);
//    timeoutTimer = new QTimer(this);
//    timeoutTimer->singleShot(3000, [=](){
//        if(this->isHidden())
//        {
//            initProDlg->close();
//            connectionClose(3);
//        }
//    });

#if TESTMODE > 0
    /* TEST INIT JSON */
    QFile initJson("init.json");
    initJson.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray ijson = initJson.readAll();

    initTools(ijson);
        /* END */
#endif

}

/**
 * @brief 获取初始化json对象，构造UI
 * @param initJsonBytes JSON字节流
 */
void MainWindow::initTools(const QByteArray &initJsonBytes)
{
    qDebug() << "Init...";
    //关闭等待计时槽函数
    disconnect(initProDlg, &QProgressDialog::canceled, this, &MainWindow::socketDisconnect);
    initProDlg->close();
    //解析对象
    QJsonParseError jsonParseError;
    QJsonDocument jsonDoucment = QJsonDocument::fromJson(initJsonBytes, &jsonParseError);

    if(jsonParseError.error != QJsonParseError::NoError)
    {
        qDebug() << initJsonBytes;
        QMessageBox::critical(NULL, "致命错误", "INIT JSON解析失败！可能的原因：" + Util::jsonParseError(jsonParseError.error));
        qApp->exit();
        return;
    }

    QJsonObject initObj = jsonDoucment.object();

    //禁用调试
    if(initObj["DisabledTools"].toBool())
    {
        QMessageBox::warning(NULL, "警告", "该设备禁止被调试！");
        connectionClose(0);
        return;
    }
    //需要认证
    if(initObj["AuthPassword"].toString() != "")
    {
        auth:
        bool ok;
        QString pwd = QInputDialog::getText(NULL, tr("认证"),
                                             tr("该设备需要认证密码："), QLineEdit::Password,
                                             "", &ok);
        if (ok && !pwd.isEmpty())
        {
            if(pwd != initObj["AuthPassword"].toString())
            {
                QMessageBox::critical(NULL, "错误", "认证失败！请重新输入");
                goto auth;
            }
            else
            {
                goto authSuccess;
            }
        }
        else
        {
            connectionClose(0);
            return;
        }
    }

    authSuccess:
    //UIJSON类别
    if(initObj["UIJsonList"].toArray().size() > 0)
    {
        QJsonArray uijsonList = initObj["UIJsonList"].toArray();
        for(int i=0;i<uijsonList.size();i++)
        {
            ui->jsonSelectComboBox->addItem(uijsonList[i].toString());
        }
    }
    else
    {
        ui->jsonSelectComboBox->addItem("ALL");
    }


    //只读
    if(initObj["ReadOnly"].toBool())
    {
        ui->tableView->setDisabled(true);
    }
    //模块
    int modulesCount = 0;
    if(initObj["Modules"].isObject())
    {
        QJsonObject modules = initObj["Modules"].toObject();
        //配置编辑器
        if(modules["ConfigEditor"].toBool())
        {
           /* 启用 */
           //单条编辑模式
           singleModify =  modules["ConfigEditor-SingleModify"].toBool();
           onlySave = modules["ConfigEditor-OnlySave"].toBool();
        }
        else
        {
            /* 禁用 */
            ui->tableView->setDisabled(true);
            qDebug() << "禁用配置编辑器";
        }
        //发送JSON到服务器
        if(modules["SendJsonToServer"].toBool())
        {
           /* 启用 */
           QPushButton *button = new QPushButton(this);
           button->setText("发送JSON数据");
           connect(button, &QPushButton::clicked, this, &MainWindow::transmissionJSON);
           ui->modulesLayout->addWidget(button);
           ui->modulesLayout->addSpacing(SPACING);
           ++modulesCount;
        }
        //保存JSON到文件
        if(modules["SaveJsonToFile"].toBool())
        {
           /* 启用 */
           QPushButton *button = new QPushButton(this);
           button->setText("保存JSON文件");
           connect(button, &QPushButton::clicked, this, &MainWindow::saveJsonToFile);
           ui->modulesLayout->addWidget(button);
           ui->modulesLayout->addSpacing(SPACING);
           ++modulesCount;
        }
        //读取JSON到配置
        if(modules["LoadJsonFormFile"].toBool())
        {
           /* 启用 */
           QPushButton *button = new QPushButton(this);
           button->setText("读取JSON文件");
           connect(button, &QPushButton::clicked, this, &MainWindow::loadJsonfromFile);
           ui->modulesLayout->addWidget(button);
           ui->modulesLayout->addSpacing(SPACING);
           ++modulesCount;
        }

        //手掌图像调试
        if(modules["PalmImagesDebug"].toBool())
        {
           /* 启用 */
           QPushButton *button = new QPushButton(this);
           button->setText("掌脉图像调试");
           connect(button, &QPushButton::clicked, this, &MainWindow::on_palmImageDebugbtn_clicked);
           ui->modulesLayout->addWidget(button);
           ui->modulesLayout->addSpacing(SPACING);
           ++modulesCount;
        }
        //手掌图像调试
        if(modules["FaceImagesDebug"].toBool())
        {
           /* 启用 */
           QPushButton *button = new QPushButton(this);
           button->setText("面部图像调试");
           connect(button, &QPushButton::clicked, this, &MainWindow::on_faceImageDebugbtn_clicked);
           ui->modulesLayout->addWidget(button);
           ui->modulesLayout->addSpacing(SPACING);
           ++modulesCount;
        }
        //上传固件
        if(modules["UploadFirmware"].toBool())
        {
           /* 启用 */
           QPushButton *button = new QPushButton(this);
           button->setText("更新固件");
           connect(button, &QPushButton::clicked, this, &MainWindow::uploadFirmware);
           ui->modulesLayout->addWidget(button);
           ui->modulesLayout->addSpacing(SPACING);
           ++modulesCount;
        }
        //同步配置
        if(modules["SyncConfig"].toBool())
        {
           /* 启用 */
           QPushButton *button = new QPushButton(this);
           button->setText("同步当前配置");
           connect(button, &QPushButton::clicked, this, &MainWindow::syncConfigDevices);
           ui->modulesLayout->addWidget(button);
           ui->modulesLayout->addSpacing(SPACING);
           ++modulesCount;
        }
        //添加弹簧
        ui->modulesLayout->addStretch();
    }
    //动态调整宽度
    this->setFixedSize(modulesCount > 0 ? 930 : 765, this->height());
    //请求UIJSON
    QByteArray defaultUIJSON = ui->jsonSelectComboBox->currentText().toUtf8();
    sendRequest(defaultUIJSON.data(), COMMAND::UIJSON);

#if TESTMODE > 0
    /* TEST UI JSON */
    QFile uiJson("system.json");
    uiJson.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray u = uiJson.readAll();

    generateTree(u);
    /* END */
#endif

    //状态栏
    QString statusBartext;
    if(socket->peerAddress().toString() == "127.0.0.1"){
        statusBartext = QString("已连接至ADB设备(%1)").arg(initObj["DeviceName"].toString());
    }else {
        statusBartext = QString("已连接至%1:%2 (%3)").arg(socket->peerAddress().toString()).arg(socket->peerPort()).arg(initObj["DeviceName"].toString());
        connectedIP = socket->peerAddress().toString();
    }
    QLabel *statusBarLabel = new QLabel(statusBartext);
    statusBarLabel->setStyleSheet("margin-left:5px;color: #004EA1");
    statusBar()->addWidget(statusBarLabel, 9);
    statusBar()->addWidget(memoryUsageStatus, 1);
    memoryUsageTimer = new QTimer(this);
    connect(memoryUsageTimer, &QTimer::timeout, this, &MainWindow::displaSystemInfo);
    memoryUsageTimer->start(1000);
    qDebug() << "Init Done.";
    //初始化结束，显示主窗口
    show();
}

/**
 * @brief setRestartState
 * @param state 状态
 */
void MainWindow::setRestartState(int &state)
{
    restart = &state;
}

/**
 * @brief 生成UI树
 * @param jsonBytes JSON字节流
 */
void MainWindow::generateTree(const QByteArray &jsonBytes)
{
    //解析JSON对象
    QJsonParseError jsonParseError;
    QJsonDocument jsonDoucment = QJsonDocument::fromJson(jsonBytes, &jsonParseError);

    if(jsonParseError.error != QJsonParseError::NoError)
    {
        QMessageBox::critical(NULL, "致命错误", "JSON UI解析失败！可能的原因：" + Util::jsonParseError(jsonParseError.error));
        return;
    }

    jsonUiObj = jsonDoucment.object();

    //树 模型
    treeModel = new QStandardItemModel(ui->treeView);
    ui->treeView->header()->hide();
    int index = 0;
    //构造树
    for(QJsonObject::Iterator it1 = jsonUiObj.begin();it1 != jsonUiObj.end();it1++)
    {
        if(it1.key() == "desc") continue; //不显示desc字段
        QString desc1 = findDescForKey(it1.key());//获取备注
        QStandardItem *item = new QStandardItem(it1.key());//model item
        item->setToolTip(desc1 == "" ? it1.key() : desc1);//设置tooltip
        if(it1.value().isObject() && !it1.value().toObject().contains("value"))
        {
            QJsonObject jsonc1Obj = it1.value().toObject();
            for(QJsonObject::Iterator it2 = jsonc1Obj.begin();it2 != jsonc1Obj.end();it2++)
            {
                if(it2.key() == "desc") continue;
                QStandardItem *itemc = new QStandardItem(it2.key());
                QString desc2 = findDescForKey(it2.key());
                itemc->setToolTip(desc2 == "" ? it2.key() : desc2);

                if(it2.value().isObject() && !it2.value().toObject().contains("value"))
                {
                    QJsonObject jsonc2Obj = it2.value().toObject();
                    for(QJsonObject::Iterator it3=jsonc2Obj.begin();it3!=jsonc2Obj.end();it3++)
                    {
                        if(it3.key() == "desc") continue;
                        QString desc3 = findDescForKey(it3.key());
                        QStandardItem *itemc2 = new QStandardItem(it3.key());
                        itemc2->setToolTip(desc3 == "" ? it3.key() : desc3);
                        itemc->appendRow(itemc2);
                    }

                }
                item->appendRow(itemc);
            }
        }
        treeModel->setItem(index++, 0, item);
    }
    //指定模型
    ui->treeView->setModel(treeModel);
    ui->treeView->expandAll();
    ui->treeView->setExpandsOnDoubleClick(false);
    //连接单击槽函数
    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::treeClicked, Qt::UniqueConnection);

    //table model
    model = new QStandardItemModel();
    ui->tableView->setModel(model);
    //modelClear();


    connect(ui->tableView, &QTableView::clicked, this, &MainWindow::tableClicked, Qt::UniqueConnection);

    waitLabel->hide();
    waitLabelTimer->stop();
    //初始默认载入第一个
    treeClicked(ui->treeView->model()->index(0, 0));

}

/**
 * @brief 将整个UIJSON序列化后由TCP Socket发出
 */
void MainWindow::transmissionJSON()
{
    //序列化
    QByteArray jsonByte = QJsonDocument(jsonUiObj).toJson(QJsonDocument::Compact);

//    QFile saveFile("sendJSON.json");
//    if(saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        saveFile.write(jsonByte);
//        saveFile.close();
//    }
    //发送
    sendRequest(jsonByte.data(), COMMAND::VALUE);
    qDebug() << "transmission FULL JSON completed";

}

/**
 * @brief 只发送修改部分的值，包含父级
 * @param keys
 * @param value
 */
void MainWindow::transmissionModify(const QStringList &keys, QString value)
{
    //构造对象
    QJsonObject singleModifyObj;
    singleModifyObj.insert("path", keys.join(","));
    singleModifyObj.insert("key", keys.last());
    singleModifyObj.insert("value", value);
    //序列化
    QByteArray jsonByte = QJsonDocument(singleModifyObj).toJson(QJsonDocument::Compact);
    //发送
    sendRequest(jsonByte, COMMAND::VALUE);
    qDebug() << "transmission Modify value[" << keys << "] completed";
}

/**
 * @brief TcpSocket接受数据的槽函数
 */
void MainWindow::socketReadData()
{
    //缓冲区内容长度
    int recvLen = socket->bytesAvailable();
    //是否需要分包读取
    if (isTcpRecvHeadOk == false)
    {
        //读取
        char *readPackageHead = (char*)malloc(sizeof(char) * recvLen + 1);
        socket->read(readPackageHead, recvLen);
        //判断开始符
        if(readPackageHead[0] == START)
        {
            //命令及长度
            cmd = readPackageHead[1];
            //qDebug("%x %x %x %x", readPackageHead[2], readPackageHead[3], readPackageHead[4], readPackageHead[2]);
            pkgLen = (uchar)readPackageHead[2];
            pkgLen = pkgLen << 8;
            pkgLen += (uchar)readPackageHead[3];
            pkgLen = pkgLen << 8;
            pkgLen += (uchar)readPackageHead[4];
            pkgLen = pkgLen << 8;
            pkgLen += (uchar)readPackageHead[5];
            //qDebug() << "PkgLen:" << pkgLen;
            crcLen = pkgLen;
            //是否读完
            isTcpRecvHeadOk = true;
            tcpRecvBlock.clear();
            //写入值
            for(int i=6;i<recvLen;i++)
            {
                //printf("%d,%X ", i, head[i]);
                tcpRecvBlock.append(readPackageHead[i]);
                //qDebug("%x %d\n", readPackageGead[i], pkgLen);
                //读完指定长度
                if(--pkgLen<=0)
                {
                    //检查校验
                    if(Util::checkCrc8Code((unsigned char*)tcpRecvBlock.data(), crcLen, readPackageHead[i+1]))
                    {
                        qDebug("recv completed, cmd: %x recv len: %d", cmd, tcpRecvBlock.size());
                        //处理数据
                        parseRecvData();
                    }
                    else
                    {
                        qDebug("CRC校验失败: 长度：%d 返回：%x 计算：%x",crcLen, (uchar)readPackageHead[i+1], Util::crc8((unsigned char*)tcpRecvBlock.data(), crcLen));
                        tcpRecvBlock.clear();
                        crcLen = 0;
                        pkgLen = 0;
                        break;
                    }
                    isTcpRecvHeadOk = false;
                    break;
                }

            }
        }
        else
        {
            qDebug("INVAILD HEAD, DROP. [0]: %x", readPackageHead[0]);
        }
        //释放
        free(readPackageHead);
    }else{
        /* 分包读取 */
        //读取
        char *readPackageBody = (char*)malloc(sizeof(char) * recvLen + 1);
        socket->read(readPackageBody, recvLen);
        //写入
        for(int i=0;i<recvLen;i++)
        {
            //printf("%d,%X ", i, body[i]);
            tcpRecvBlock.append(readPackageBody[i]);
            //读完指定长度
            if(--pkgLen<=0)
            {
                //检查校验
                if(Util::checkCrc8Code((unsigned char*)tcpRecvBlock.data(), crcLen, readPackageBody[i+1]))
                {
                    qDebug() << "recv completed, recv len:" << tcpRecvBlock.size();
                    //数据处理
                    parseRecvData();
                    isTcpRecvHeadOk = false;
                    break;
                }
                else
                {
                    qDebug("CRC校验失败: 长度：%d 返回：%x 计算：%x",crcLen, (uchar)readPackageBody[i+1], Util::crc8((unsigned char*)tcpRecvBlock.data(), crcLen));
                    tcpRecvBlock.clear();
                    crcLen = 0;
                    pkgLen = 0;
                    break;
                }
            }

        }
        //释放
        free(readPackageBody);
    }
}

/**
 * @brief 监听socket是否断开
 */
void MainWindow::socketDisconnect()
{
    connectionClose(2);
}

/**
 * @brief 从Util类中获取到系统信息渲染到UI上
 */
void MainWindow::displaSystemInfo()
{
    SystemInfo info;
    Util::getSystemInfo(info);
    memoryUsageStatus->setText(QString("CPU: %1%  内存: %2 Mb").arg(info.cpuUsage).arg(QString::number(info.memoryUsage, 10, 2)));
}

/**
 * @brief 解析接受数据
 */
void MainWindow::parseRecvData()
{
    //数据长度
    int recvLen = tcpRecvBlock.size();
    //解析命令
    switch (cmd) {
    case COMMAND::INIT: //初始化
        initTools(tcpRecvBlock);
        break;

    case COMMAND::UIJSON://UI JSON
        generateTree(tcpRecvBlock);
        break;

    case COMMAND::IMAGES_STREAM://图片
    {
        sendRespones();
        char *tcpRecvChar, *imgData;
        char type;
        int width, height;
        tcpRecvChar = tcpRecvBlock.data();
        //长，宽
        width = ((uchar*)tcpRecvChar)[0];// 0x2
        width = width  << 8;
        width += ((uchar*)tcpRecvChar)[1];
        height = ((uchar*)tcpRecvChar)[2];
        height = height << 8;
        height += ((uchar*)tcpRecvChar)[3];
        type = tcpRecvChar[4];
        //图像数据
        imgData = (char*)malloc(sizeof(char) * recvLen - 4);
        memcpy(imgData, tcpRecvChar+5, recvLen-5);

        qDebug() << "Detected Img, size:" << width << height;
        //构造图像对象
        QImage image((uchar*)imgData, width, height, QImage::Format_Indexed8);
        //判断图像类别
        switch (type) {
        case IMAGETYPE::PALM:
        {
            if(palmImagePanel==nullptr) break;
            //渲染至对应UI
            palmImagePanel->renderImage(image);
            qDebug() << "Is Palm RenderImg";
            break;
        }
        case IMAGETYPE::FACE:
        {
            if(faceImagePanel==nullptr) break;
            faceImagePanel->renderImage(image);
            qDebug() << "Is Face RenderImg";
            break;
        }
        default:
            qDebug("Unknow IMAGETYPE %c", type);
            break;
        }
        //释放内存
        free(imgData);
        break;

    }
    case COMMAND::VALUE: //改值
    {
        QString error;
        if(!checkReplySuccess(tcpRecvBlock, error))
        {
            QString errorMsg = QString("改值失败！\n返回错误信息：%1").arg(error);
            QMessageBox::critical(this, "错误", errorMsg);
            return;
        }
        else
        {
            QMessageBox::information(this, "成功", "传输成功");
            return;
        }
        break;
    }
    case COMMAND::FIRMWARE_START: //准备上传
    {
        QString error;
        if(checkReplySuccess(tcpRecvBlock, error))
            //开始上传
            uploadByFTP();
        else
        {
            QString errorMsg = QString("远程资源服务器启动失败！\n返回错误信息：%1").arg(error);
            QMessageBox::critical(this, "错误", errorMsg);
            return;
        }
        break;
    }
    case COMMAND::IMAGES_ENABLED: //启用流
    {
        QString error;
        if(!checkReplySuccess(tcpRecvBlock, error))
        {
            QString errorMsg = QString("启用图像流失败！\n返回错误信息：%1").arg(error);
            QMessageBox::critical(this, "错误", errorMsg);
            return;
        }
        break;
    }
    case COMMAND::IMAGES_DISABLED: //关闭流
    {
        QString error;
        if(!checkReplySuccess(tcpRecvBlock, error))
        {
            QString errorMsg = QString("关闭图像流失败！\n返回错误信息：%1").arg(error);
            QMessageBox::critical(this, "错误", errorMsg);
            return;
        }
        break;
    }
    default: //未知命令
        qDebug("UnKnow CMD: %c", cmd);
        break;
    }
}

/**
 * @brief 数据接受完毕后发送的响应，防止服务端连续
 */
void MainWindow::sendRespones()
{
    sendRequest(COMMAND::RESPONE);
    //qDebug() << "send respones";
}

/**
 * @brief 将内容封装至结构体再又socket对象发出
 * @param body
 * @param cmd
 */
void MainWindow::sendRequest(const char *body, COMMAND cmd)
{
    //协议报文
    pd_Package_t pkg;
    int lengh = (int)strlen(body);
    pkg.start = START; //起始符
    pkg.cmd = cmd; //命令
    pkg.len[0] = lengh>>24; //长度
    pkg.len[1] = lengh>>16;
    pkg.len[2] = lengh>>8;
    pkg.len[3] = lengh;
    pkg.body = (char*)body; //内容
    pkg.check = Util::crc8((unsigned char*)body, lengh); //校验
    int state = socket->write(structToStream(pkg)); //发送
    qDebug() << "Send Size:" << state;
    if(state < 0) //发送失败
        qDebug() << "TCP发送失败, 设备未连接";
    //刷新缓冲区
    socket->flush();
}

/**
 * @brief 将随机字符（8位）封装至结构体再又socket对象发出
 * @param cmd
 */
void MainWindow::sendRequest(COMMAND cmd)
{
    //获取随机字符
    const char chrs[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char body[9] = "00000000";

    int chrsSize = sizeof(chrs);
    int randomx = 0;
    for (int i = 0; i < 8; ++i)
    {
       randomx= rand() % (chrsSize - 1);
       body[i] = chrs[randomx];
    }
    //协议报文
    pd_Package_t pkg;
    int lengh = (int)strlen(body);
    pkg.start = START;
    pkg.cmd = cmd;
    pkg.len[0] = lengh>>24;
    pkg.len[1] = lengh>>16;
    pkg.len[2] = lengh>>8;
    pkg.len[3] = lengh;
    pkg.body = (char*)body;
    pkg.check = Util::crc8((unsigned char*)body, lengh);
    int state = socket->write(structToStream(pkg));
    if(state < 0)
        qDebug() << "TCP发送失败, 设备未连接";
    socket->flush();
}

/**
 * @brief 检查回复报文是否有效
 * @param jsonBytes
 * @param error
 * @return bool
 */
bool MainWindow::checkReplySuccess(QByteArray jsonBytes, QString &error)
{
    //解析JSON对象
    QJsonParseError jsonParseError;
    QJsonDocument jsonDoucment = QJsonDocument::fromJson(jsonBytes, &jsonParseError);

    if(jsonParseError.error != QJsonParseError::NoError)
    {
        qDebug() << "检查回复报文内容 解析失败" << jsonBytes;
        error = "报文解析失败错误";
        return false;
    }
    QJsonObject replyObj = jsonDoucment.object();
    if(!replyObj["state"].isNull() && replyObj["state"].toInt() == 0)
        return true;
    else
    {
        qDebug() << "检查回复报文内容 校验失败" << jsonBytes;
        error = replyObj["message"].toString();
        return false;
    }

}

/**
 * @brief 把TCP报文结构体转为字节流以便发送
 * @param pkg
 * @return QByteArray 字节流
 */
QByteArray MainWindow::structToStream(pd_Package pkg)
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
    qDebug() << "sendBlock：size:" << sendBlock.size() << "body:" << sendBlock.toHex();

    return sendBlock;
}

/**
 * @brief 断开socket连接后关闭窗口
 * @param state 状态
 */
void MainWindow::connectionClose(int state)
{
    /*
      1:手动断开
      2:被动断开
      3:连接超时
    */
    initProDlg->close();
    //关闭socket
    if(socket->isOpen())
    {
        socket->disconnectFromHost();
        socket->close();
    }
    if(!isSocketManualClose) {
        *restart = state;
        isSocketManualClose = false;
    }
    qDebug() << "disconnected";
    //关闭窗口
    this->close();
}

/**
 * @brief 实例化掌脉调试窗口，并显示
 */
void MainWindow::on_palmImageDebugbtn_clicked()
{
    //防重
    if(palmImagePanel!=nullptr) return;
    //开启图像流
    sendRequest("P", COMMAND::IMAGES_ENABLED);
    qDebug() << "open Palm Stream";
    //实例化
    palmImagePanel = new ImageDisplay();
    palmImagePanel->setDisplayType("palm", true);
    palmImagePanel->setWindowTitle("PDTools-掌脉图像调试");
    palmImagePanel->setWindowIcon(QIcon(":/pixmap/images/favicon.ico"));
    //连接槽函数
    connect(palmImagePanel, &ImageDisplay::controlImageDebugCMD, this, &MainWindow::imageDebugControl);
    //显示
    palmImagePanel->show();
    //palmImagePanel->renderImage(QImage("F:\\test.jpg"));
}

/**
 * @brief实例化面容调试窗口，并显示
 */
void MainWindow::on_faceImageDebugbtn_clicked()
{
    //防重
    if(faceImagePanel!=nullptr) return;
    //开启图像流
    sendRequest("F", COMMAND::IMAGES_ENABLED);
    qDebug() << "open Face Stream";
    //实例化
    faceImagePanel = new ImageDisplay;
    faceImagePanel->setDisplayType("face");
    faceImagePanel->setWindowTitle("PDTools-面部图像调试");
    faceImagePanel->setWindowIcon(QIcon(":/pixmap/images/favicon.ico"));
    //连接槽函数
    connect(faceImagePanel, &ImageDisplay::controlImageDebugCMD, this, &MainWindow::imageDebugControl);
    //显示
    faceImagePanel->show();
}

/**
 * @brief 由调试窗口发来的控制命令来调用主窗口的事件
 * @param type
 * @param cmd
 */
void MainWindow::imageDebugControl(const QString &type, const QString &cmd)
{
    //捕获
    if(cmd == "capture")
    {
        if(type == "palm")
        {
            sendRequest("P", COMMAND::IMAGES_CAPTURE);
        }
        if(type == "face")
        {
            sendRequest("F", COMMAND::IMAGES_CAPTURE);
        }
    }
    //关闭
    if(cmd == "close")
    {
        if(type == "palm")
        {
            //关闭图像流
            sendRequest("P", COMMAND::IMAGES_DISABLED);
            //断开槽函数
            disconnect(palmImagePanel, &ImageDisplay::controlImageDebugCMD, this, &MainWindow::imageDebugControl);
            delete palmImagePanel;
            palmImagePanel = nullptr;
        }
        if(type == "face")
        {
            sendRequest("F", COMMAND::IMAGES_DISABLED);
            disconnect(faceImagePanel, &ImageDisplay::controlImageDebugCMD, this, &MainWindow::imageDebugControl);
            delete faceImagePanel;
            faceImagePanel = nullptr;
        }

    }
}

/**
 * @brief 菜单栏退出程序功能
 */
void MainWindow::on_quitWindows_triggered()
{
    this->close();
}

/**
 * @brief 菜单栏关闭连接功能
 */
void MainWindow::on_closeConnection_triggered()
{
    connectionClose(1);
}

/**
 * @brief 菜单栏关于Qt功能
 */
void MainWindow::on_aboutQt_triggered()
{
    QMessageBox::aboutQt(this, "aboutQt");
}

/**
 * @brief 菜单栏软件信息功能
 */
void MainWindow::on_infomation_triggered()
{
    Util::getSoftwareInfomation();
}

/**
 * @brief 菜单栏检查更新功能
 */
void MainWindow::on_checkUpdate_triggered()
{
    Util::checkUpdate("PDTools", version, true);
}

/**
 * @brief 等待接受数据动画效果
 */
void MainWindow::waitLabelTextChange()
{
    QString currentText = waitLabel->text();
    int i = waitLabelText->indexOf(currentText);
    if(i == waitLabelText->size()-1)
    {
        //末尾重置
        waitLabel->setText(waitLabelText->at(0));
    }
    else
    {
        waitLabel->setText(waitLabelText->at(i+1));
    }
}

/**
 * @brief 将内存中的UIJSON对象保存至本地
 */
void MainWindow::saveJsonToFile()
{
    //序列化JSON对象
    QByteArray jsonByte= QJsonDocument(jsonUiObj).toJson(QJsonDocument::Compact);
    QString filename = QString("%1").arg(ui->jsonSelectComboBox->currentText());
    //保存位置
    QString filePath = QFileDialog::getSaveFileName(this, "请选择保存位置", "./"+filename);
    if(filePath.isEmpty()) return;
    else{
        QFile saveFile(filePath);
        if(saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            saveFile.write(jsonByte);
            saveFile.close();
        }
        else
        {
            QMessageBox::critical(this, "错误", "写入文件失败，请检查目录权限设置");
            return;
        }
    }
}

/**
 * @brief 读取本地的JSON文件来替换UIJSON对象
 */
void MainWindow::loadJsonfromFile()
{
    //读取JSON到配置
    QString filePath = QFileDialog::getOpenFileName(this, "请选择文件", "./");
    if(filePath.isEmpty()) return;
    else{
        QFile saveFile(filePath);
        if(saveFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            //读取序列化
            QByteArray jsonByteArray = saveFile.readAll();
            //解析
            QJsonParseError jsonParseError;
            QJsonDocument jsonDoucment = QJsonDocument::fromJson(jsonByteArray, &jsonParseError);
            if(jsonParseError.error == QJsonParseError::NoError)
            {
                //重载结构树
                generateTree(jsonByteArray);
            }
            else
            {
                QMessageBox::critical(NULL, "错误", "JSON解析失败！可能的原因：" + Util::jsonParseError(jsonParseError.error));
                return;
            }

        }
        else
        {
            QMessageBox::critical(this, "错误", "读取文件失败，请检查文件是否有效");
            return;
        }
    }

}

/**
 * @brief 菜单栏配置文件功能
 * @note 用cmd调用notepad.exe来打开配置文件
 */
void MainWindow::on_action_triggered()
{
    QProcess *pro;
    pro->startDetached("notepad.exe config.ini");
}

/**
 * @brief 本地选择一个文件，并启动服务端
 */
void MainWindow::uploadFirmware()
{
    //选择文件
    selectFilePath = QFileDialog::getOpenFileName(this, "选择固件", uploadBeforePath, "Firmware (*.zip)");
    if(!selectFilePath.isEmpty())
    {
        uploadBeforePath = selectFilePath.left(selectFilePath.lastIndexOf("/"));
        QString fileName = selectFilePath.right(selectFilePath.size() - selectFilePath.lastIndexOf("/") - 1);
        if(!fileName.toLower().contains("update") && !fileName.toLower().contains("airwave"))
        {
            QMessageBox::critical(this, "提示", "请选择正确的固件包");
            return;
        }
        sendRequest(COMMAND::FIRMWARE_START);
        //初始化进度条

        uploadFirmwareProgressBar->setFixedSize(500, 40);
        uploadFirmwareProgressBar->setWindowTitle("正在建立连接...");
        uploadFirmwareProgressBar->setWindowFlags(Qt::WindowCloseButtonHint|Qt::WindowStaysOnTopHint);
        uploadFirmwareProgressBar->setValue(0);
        uploadFirmwareProgressBar->setMaximum(0);
        uploadFirmwareProgressBar->setStyleSheet("font-size:25px");
        uploadFirmwareProgressBar->show();
    }
}

/**
 * @brief 显示一个窗口显示扫描到的局域网设备
 */
void MainWindow::syncConfigDevices()
{
    //实例化窗口
    syncWidget = new QDialog;
    int width = 300;
    int height = 300;
    int padding = 10;
    syncWidget->setWindowTitle(QString("同步配置 -> %1").arg(ui->jsonSelectComboBox->currentText()));
    syncWidget->setFixedSize(width, height);
    syncWidget->setAttribute(Qt::WA_DeleteOnClose);
    syncWidget->move(this->x()+(this->width()-syncWidget->width()) / 2,
                     this->y()+(this->height()-syncWidget->height()) / 2);
    QGroupBox *gp = new QGroupBox(syncWidget);
    gp->setGeometry(padding, padding, width-padding*2, height-60);
    gp->setTitle("设备扫描");
    //实例化扫描列表
    boardCastListWidget = new BoardCastListWidget(gp);
    boardCastListWidget->setGeometry(5, 15, gp->width()-10, gp->height()-20);
    boardCastListWidget->initListWidget(true);
    boardCastListWidget->hideConnected(connectedIP);
    QWidget *buttons = new QWidget(syncWidget);
    buttons->setGeometry(padding, height - 40, syncWidget->width()-padding*2, 30);
    QHBoxLayout *buttonsLayout = new QHBoxLayout(buttons);
    buttonsLayout->setMargin(0);
    buttonsLayout->setSpacing(3);
    //同步选中 按钮
    QPushButton *syncChecked = new QPushButton("同步选中设备", syncWidget);
    buttonsLayout->addWidget(syncChecked);
    //同步所有 按钮
    QPushButton *syncAll = new QPushButton("同步所有设备", syncWidget);
    buttonsLayout->addWidget(syncAll);
    //重新搜索 按钮
    QPushButton *rescan = new QPushButton("重新搜索", syncWidget);
    buttonsLayout->addWidget(rescan);
    buttons->setLayout(buttonsLayout);
    //同步选择 匿名槽函数
    connect(syncChecked, &QPushButton::clicked, [=](){
        //获取选中
        QList<QString> selection =  boardCastListWidget->getSelectRow();
        syncDevices(selection);
    });
    //同步所有 匿名槽函数
    connect(syncAll, &QPushButton::clicked, [=](){
        //获取所有
        QList<QString> selection =  boardCastListWidget->getSelectRow(true);
        syncDevices(selection);
    });
    //重新搜索 匿名槽函数
    connect(rescan, &QPushButton::clicked, [=](){
        boardCastListWidget->rescanDevices();
    });

    //模态显示
    syncWidget->exec();
}

/**
 * @brief 将内存中的UIJSON同步给局域网其他设备
 * @param selection 选择的列
 */
void MainWindow::syncDevices(QList<QString> selection)
{
    //用来同步的socket
    QTcpSocket *syncSocket = new QTcpSocket(this);
    for(int i=0;i<selection.size();i++)
    {
        QString syncState = "FAILED";
        QStringList t = selection[i].split("&");
        QString ip = t[0].split(":")[0];
        int port = t[0].split(":")[1].toInt();
        int row = t[1].toInt();
        //连接
        syncSocket->connectToHost(ip, port);
        if(syncSocket->waitForConnected(1500))
        {
            /* 连接成功 */
            //json
            char *body;
            QJsonObject syncJson;
            syncJson.insert("name", ui->jsonSelectComboBox->currentText());
            syncJson.insert("json", jsonUiObj);
            QByteArray jsonByte = QJsonDocument(syncJson).toJson(QJsonDocument::Compact);
            body = jsonByte.data();
            //pkg
            pd_Package_t pkg;
            int lengh = (int)strlen(body);
            pkg.start = START;
            pkg.cmd = COMMAND::SYNC;
            pkg.len[0] = lengh>>24;
            pkg.len[1] = lengh>>16;
            pkg.len[2] = lengh>>8;
            pkg.len[3] = lengh;
            pkg.body = (char*)body;
            pkg.check = Util::crc8((unsigned char*)body, lengh);
            int state = syncSocket->write(structToStream(pkg));
            if(state > 0)
            {
                //发送成功
                syncSocket->flush();

                if(syncSocket->waitForReadyRead(1500))
                {
                    //接受到数据
                    QByteArray datagram = syncSocket->readAll();
                    if(datagram.size() >= LEN_HEAD_PKG && datagram[0] == START && datagram[1] == (char)COMMAND::SYNC)
                    {
                        //包头正确
                        int pkgLen;
                        unsigned char crc;

                        //length
                        pkgLen = datagram[2];
                        pkgLen = pkgLen << 8;
                        pkgLen += datagram[3];
                        pkgLen = pkgLen << 8;
                        pkgLen += datagram[4];
                        pkgLen = pkgLen << 8;
                        pkgLen += datagram[5];
                        //body
                        QByteArray bodyJsonByte;
                        for(int i=0;i<pkgLen;i++) bodyJsonByte.insert(i, datagram[i+6]);
                        //crc
                        crc = datagram[datagram.size()-1];
                        if(Util::checkCrc8Code((unsigned char *)bodyJsonByte.data(), pkgLen, crc))
                        {
                            //校验成功
                            //json parse
                            QJsonParseError json_parse_error;
                            QJsonDocument json_doucment = QJsonDocument::fromJson(bodyJsonByte, &json_parse_error);

                            if(json_parse_error.error == QJsonParseError::NoError)
                            {
                                QJsonObject resIPObj = json_doucment.object();
                                if(resIPObj["state"].toInt() == 0)
                                {
                                    syncState = "SUCCESS";
                                }else qDebug() << "PKG BODY ERROR" << resIPObj;
                            }
                            else
                            {
                                qDebug() << "Sync Respone Json Parse Error:" << Util::jsonParseError(json_parse_error.error);
                                qDebug() << bodyJsonByte;
                            }
                        }else qDebug() << "PKG CHECK ERROR";

                    }else qDebug() << "PKG HEAD ERROR" <<datagram.size() << datagram[0] << datagram[1] << (char)COMMAND::SYNC;

                }
                else
                {
                    syncState = "TIMEOUT";
                }
            }
            else
            {
                syncState = "SENDERR";
            }
        }
        else
        {
            syncState = "TIMEOUT";
        }

        boardCastListWidget->setSyncResult(row, syncState);
        syncSocket->disconnectFromHost();
        syncSocket->waitForDisconnected(1000);
    }
    delete syncSocket;
}

/**
 * @brief UIJSON列表选择 事件
 * @param arg1
 */
void MainWindow::on_jsonSelectComboBox_currentIndexChanged(const QString &arg1)
{
    waitLabel->show();
    waitLabelTimer->start(200);
    sendRequest(arg1.toUtf8().data(), COMMAND::UIJSON);
}

/**
 * @brief FTP上传
 */
void MainWindow::uploadByFTP()
{
    //打开文件并序列化
    QByteArray totalData;
    QFile selectFile(selectFilePath);
    if (!selectFile.open(QIODevice::ReadOnly))
    {
      QMessageBox::critical(this, "错误", "序列化文件失败！");
      return;
    }
    totalData = selectFile.readAll();
    selectFile.close();
    //进度条 改变
    uploadFirmwareProgressBar->setWindowTitle("正在上传...");
    uploadFirmwareProgressBar->setValue(0);
    uploadFirmwareProgressBar->setMaximum(100);

    //网络管理器
    QNetworkAccessManager *accessManager = new QNetworkAccessManager(this);
    accessManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
    //路径
    QString fileName = selectFilePath.right(selectFilePath.size() - selectFilePath.lastIndexOf("/") - 1);
    QUrl url(QString("ftp://%1//%2").arg(socket->peerAddress().toString()).arg(fileName));
    url.setPort(21);
    url.setUserName("FtpUser");
    url.setPassword("Ftp123456");
    //请求
    uploadStatus = true;
    QNetworkRequest request(url);
    ftpReply = accessManager->put(request, totalData);
    connect(accessManager, &QNetworkAccessManager::finished, this, &MainWindow::replyFTPFinished);
    connect(ftpReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(loadFTPError(QNetworkReply::NetworkError)));
    connect(ftpReply, &QNetworkReply::uploadProgress, this, &MainWindow::loadFTPProgress);
}

/**
 * @brief FTP上传结束
 * @param reply 请求对象
 */
void MainWindow::replyFTPFinished(QNetworkReply *reply)
{
    if(uploadStatus)
    {
        uploadFirmwareProgressBar->hide();
        QMessageBox::information(this, "提示", "上传成功");
        QString fileName = selectFilePath.right(selectFilePath.size() - selectFilePath.lastIndexOf("/") - 1);
        sendRequest(fileName.toUtf8().data(), COMMAND::FIRMWARE_END);
        qDebug() << "上传完成";
    }
}

/**
 * @brief FTP上传错误
 */
void MainWindow::loadFTPError(QNetworkReply::NetworkError)
{
    uploadFirmwareProgressBar->hide();
    uploadStatus = false;
    QMessageBox::critical(this, "上传失败", ftpReply->errorString());
}

/**
 * @brief FTP上传进度
 * @param bytesSent 已传输字节
 * @param bytesTotal 总字节
 */
void MainWindow::loadFTPProgress(qint64 bytesSent, qint64 bytesTotal)
{
    int uploadPrecent = (int)(((double)bytesSent / bytesTotal) * 100);
    QString strPrecent = QString::number(uploadPrecent);
    uploadFirmwareProgressBar->setValue(uploadPrecent);
    //qDebug() << strPrecent;
}

/**
 * @brief 检索树搜索框 监听值改变槽函数
 * @param arg1 输入字节
 */
void MainWindow::on_searchTreelineEdit_textChanged(const QString &arg1)
{
    QList<QStandardItem *> searchItems = treeModel->findItems(arg1, Qt::MatchContains | Qt::MatchRecursive);
    QItemSelectionModel* selectionModel = ui->treeView->selectionModel();
    if(searchItems.size()>0)
    {
        selectionModel->select(searchItems[0]->index(), QItemSelectionModel::SelectCurrent);
    }
}

void MainWindow::on_action_U_triggered()
{
    QDesktopServices::openUrl(QUrl("http://192.168.0.230:8850/softwares/?query_id=6"));
}
