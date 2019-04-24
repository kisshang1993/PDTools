/*************************************************
    Copyright: 2018-2019 Melux
    Author: ChengHang
    Date:2018-05-14
    Description: 图像调试窗口
**************************************************/
#include "imagedisplay.h"
#include "ui_imagedisplay.h"

/**
 * @brief 构造函数
 * @param parent
 */
ImageDisplay::ImageDisplay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageDisplay)
{
    ui->setupUi(this);
    //初始化
    saveFormatRbGroup = new QButtonGroup(this);
    saveFormatRbGroup->addButton(ui->rb_format_jpg, 1);
    saveFormatRbGroup->addButton(ui->rb_format_bmp, 2);
    saveFormatRbGroup->setExclusive(true);
    connect(saveFormatRbGroup, SIGNAL(buttonClicked(int)), this, SLOT(on_save_format_rbGroup_Change(int)));
    ui->rb_format_jpg->setChecked(true);
    scene = new QGraphicsScene(this);
    ui->graphicsView->centerOn(ui->graphicsView->mapToScene(QPoint(319, 237)));
    //读取配置文件
    QFile cfg("config.ini");
    if(!cfg.exists())
    {
        QMessageBox::critical(NULL, "错误", "缺失配置文件！请检查目录下是否存在config.ini");
        return;
    }
    config = new QSettings("config.ini", QSettings::IniFormat);
    ui->grayRef->setText(config->value("Display/grayRef").toString());
    ui->minRef->setText(config->value("Display/minRef").toString());
    ui->maxRef->setText(config->value("Display/maxRef").toString());
}

/**
 * @brief 清理指针
 */
ImageDisplay::~ImageDisplay()
{
    delete saveFormatRbGroup;
    delete ui;
}

/**
 * @brief 图像调整尺寸
 */
void ImageDisplay::imageResize()
{
    int minW = minimumWidth();
    int minH = minimumHeight();
    int changeW = width() - minW ;
    int changeH = height() - minH;
    ui->graphicsView->setGeometry(170, 30, ui->canvas->minimumWidth()+changeW, ui->canvas->minimumHeight()+changeH);
    //scale_w = ui->canvas->pixmap()->width() / (double)tempImg.width();
    //scale_h = ui->canvas->pixmap()->height() / (double)tempImg.height();
    //qDebug() << scale_w << scale_h;
    //ui->canvas->setGeometry(170, 30, ui->canvas->minimumWidth()+change_w, ui->canvas->minimumHeight()+change_h);

//    if(!ui->canvas->pixmap()==0)
//    {
//        ui->canvas->setPixmap(QPixmap::fromImage(ScaleImage2Label(ui->canvas->pixmap()->toImage(), ui->canvas)));
//    }
}

/**
 * @brief 渲染图像
 * @param image
 */
void ImageDisplay::renderImage(QImage &image)
{
    //临时图像
    tempImg = image.copy();
    tempMat = QImageToMat(image);
    int grayValue, minValue, maxValue;
    //计算灰度
    if(this->displayGray)
    {
        getGrayBenchmark(tempMat, grayValue, minValue, maxValue);
        ui->grayValue->setText(QString::number(grayValue));
        ui->minValue->setText(QString::number(minValue));
        ui->maxValue->setText(QString::number(maxValue));
    }
    //渲染在绘图区域中
    scene->clear();
    scene->addPixmap(QPixmap::fromImage(image));
    imageResize();
    ui->graphicsView->setScene(scene);
    ui->graphicsView->show();
    //是否自动保存
    if(autoSave)
    {
        QString savePath = QString("%1/%2-%3.%4").arg(autoSavePath).arg(imgType).arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz")).arg(getSaveFormat());
        //保存图像
        saveCurrentImage(savePath);
    }
}

/**
 * @brief 设置显示模式
 * @param type 类别
 * @param displayGray 时候计算灰度
 */
void ImageDisplay::setDisplayType(const QString &type, bool displayGray)
{
    imgType = type;
    this->displayGray = displayGray;
    qDebug() << "Images Debug:" << imgType << "displayGray:" <<this->displayGray;
}

/**
 * @brief 窗口尺寸调整响应事件（重写）
 * @param event
 */
void ImageDisplay::resizeEvent(QResizeEvent *event)
{
    imageResize();
}

/**
 * @brief 窗口关闭响应事件（重写），发送信号
 * @param event
 */
void ImageDisplay::closeEvent(QCloseEvent *event)
{
    //发送信号
    emit controlImageDebugCMD(imgType, "close");
}

/**
 * @brief 获取保存格式
 * @return 图像格式
 */
QString ImageDisplay::getSaveFormat()
{
    QString format;
    switch (saveFormatRbGroup->checkedId()) {
    case 1:
        format = "jpg";
        break;
    case 2:
        format = "bmp";
        break;
    default:
        break;
    }

    return format;
}

/**
 * @brief 保存按键 单击槽函数
 */
void ImageDisplay::on_saveButton_clicked()
{
    //图片不为空
    if(tempImg.isNull())
    {
        QMessageBox::critical(this, "错误", "请等待接受数据");
        return;
    }
    //保存路径
    QString filename = QString("%1-%2.%3").arg(imgType).arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz")).arg(getSaveFormat());
    QString filePath = QFileDialog::getSaveFileName(this, "请选择保存位置", "./"+filename);
    if(filePath.isEmpty()) return;
    else{
        //保存
        saveCurrentImage(filePath);
    }
}

/**
 * @brief 自动保存按键 单击槽函数
 */
void ImageDisplay::on_autosaveButton_clicked()
{
    //图片不为空
    if(tempImg.isNull())
    {
        QMessageBox::critical(this, "错误", "请等待接受数据");
        return;
    }
    //是否自动保存
    if(!autoSave)
    {
        if(autoMakeDirState)
        {
            //自动保存路径
            autoSavePath = "./" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
            //判断文件夹是否存在
            Util::isDirExist(autoSavePath);
            ui->autosaveButton->setText("关闭自动保存");
            autoSave = true;
        }
        else
        {
            autoSavePath = QFileDialog::getExistingDirectory(this, "请选择保存位置", "./");
            if(autoSavePath.isEmpty()) return;
            else{
                ui->autosaveButton->setText("关闭自动保存");
                autoSave = true;
            }

        }
    }
    else
    {
        ui->autosaveButton->setText("启用自动保存");
        autoSave = false;
    }

}

/**
 * @brief 捕获按键 单击槽函数
 */
void ImageDisplay::on_captureButton_clicked()
{
    //发送信号
    emit controlImageDebugCMD(imgType, "capture");
}

/**
 * @brief 保存当前图像
 * @param savePath 保存路径
 * @param autoMode 是否自动保存
 */
void ImageDisplay::saveCurrentImage(const QString &savePath, bool autoMode)
{
    //图像不为空
    if(tempImg.isNull()) return;
    //自动模式
    if(autoMode)
    {
        QString filename = QString("%1-%2.%3").arg(imgType).arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz")).arg(getSaveFormat());
        QPixmap::fromImage(tempImg).save(savePath + "/" + filename);
    }
    else
    {
        QPixmap::fromImage(tempImg).save(savePath);
    }
}

/**
 * @brief 改变保存格式
 * @param id 保存格式按钮组索引
 */
void ImageDisplay::changeSaveFormat(const int id)
{
    saveFormatRbGroup->button(id)->setChecked(true);
}

/**
 * @brief 开关自动保存
 * @param savePath
 */
void ImageDisplay::changeAutoSaveFunc(const bool state, const QString &savePath)
{
    autoSave = state;
    autoSavePath = savePath;
    if(!autoSave)
    {
        ui->autosaveButton->setText("关闭自动保存");
        autoSave = true;
    }
    else
    {
        ui->autosaveButton->setText("启用自动保存");
        autoSave = false;
    }
}

/**
 * @brief 改变自动保存路径
 * @param state 自动保存
 */
void ImageDisplay::changeAutoMakeDirFunc(const bool state)
{
    autoMakeDirState = state;
    ui->autoMakeDir->setChecked(autoMakeDirState);
}

/**
 * @brief 改变自动保存路径
 * @param image 原图像
 * @return Mat 转换后的图像
 */
Mat ImageDisplay::QImageToMat(const QImage &image)
{
    cv::Mat mat;
    switch (image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, CV_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
    case QImage::Format_Grayscale8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat;

}

/**
 * @brief I计算灰度值
 * @param img 图像
 * @param grayValue 灰度值
 * @param minValue 最小值
 * @param maxValue 最大值
 * @return Point 中心点
 * @author Melux XueYongBo
 */
cv::Point ImageDisplay::getGrayBenchmark(cv::Mat img, int &grayValue, int &minValue, int &maxValue)
{
    if (img.empty())
        return cv::Point();

    cv::Mat element = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::Mat maskImg = img > 30;
    morphologyEx(maskImg, maskImg, cv::MORPH_OPEN, element, cv::Point(-1, -1), 3);

    cv::Moments moment;//矩
    moment = moments(maskImg, false);

    cv::Point centre = cv::Point(img.cols / 2, img.rows / 2);
    if (moment.m00 > 2.2250738585072014e-308)
    {
        centre.x = moment.m10 / moment.m00;
        centre.y = moment.m01 / moment.m00;
    }

    cv::Rect roi = cv::Rect(centre.x - 100, centre.y - 100, 200, 200);

    roi.x = roi.x < 0 ? 0 : roi.x;
    roi.y = roi.y < 0 ? 0 : roi.y;
    roi.x = roi.x + roi.width > img.cols ? img.cols - roi.width : roi.x;
    roi.y = roi.y + roi.height > img.rows ? img.rows - roi.height : roi.y;

    cv::Mat imgROI = img(roi);
    cv::Mat maskImgROI = maskImg(roi);

    double min = 0, max = 0;
    cv::minMaxIdx(imgROI, &min, &max, NULL, NULL, maskImgROI);
    grayValue = cv::mean(imgROI, maskImgROI)[0];
    minValue = min;
    maxValue = max;
    return centre;
}

/**
 * @brief 自动创建文件夹复选框 单击槽函数
 * @param checked
 */
void ImageDisplay::on_autoMakeDir_clicked(bool checked)
{
    autoMakeDirState = checked;
}

/**
 * @brief 操作帮助按键 单击槽函数
 */
void ImageDisplay::on_pushButton_clicked()
{
    QDialog dlg(this);
    dlg.setFixedSize(300,240);
    dlg.setWindowTitle("显示区域操作帮助");

    QTextBrowser textBrowser(&dlg);
    textBrowser.setGeometry(0, 0, dlg.width(), dlg.height());
    textBrowser.setHtml("<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN' 'http://www.w3.org/TR/REC-html40/strict.dtd'><html><head><meta name='qrichtext' content='1' /><style type='text/css'>body{padding: 40px;} p, li { white-space: pre-wrap; }</style></head><body style=' font-family:'SimSun'; font-size:8.25pt; font-weight:400; font-style:normal;'><p style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><span style=' font-size:9pt; font-weight:600; text-decoration: underline;'>图片操作按键</span></p><p style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:9pt; font-weight:600;'><br /></p><p style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><span style=' font-size:10pt; font-weight:600; color:#065ebb;'>鼠标拖曳/方向键</span></p><p style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><span style=' font-size:10pt;'>移动图片</span></p><p style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:10pt;'><br /></p><p style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><span style=' font-size:10pt; font-weight:600; color:#065ebb;'>鼠标滚轮/W &amp; S</span></p><p style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><span style=' font-size:10pt;'>缩放图片</span></p><p style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:10pt;'><br /></p><p style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><span style=' font-size:10pt; font-weight:600; color:#065ebb;'>A / D</span></p><p style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><span style=' font-size:10pt;'>旋转图片</span></p></body></html>");

    dlg.exec();
}

