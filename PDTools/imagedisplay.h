/*************************************************
Copyright: 2018-2019 Melux
Author: ChengHang
Date:2018-05-14
Description: 图像调试窗口
**************************************************/
#ifndef IMAGEDISPLAY_H
#define IMAGEDISPLAY_H

#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QButtonGroup>
#include <QSettings>
#include <QStatusBar>
#include <QDebug>
#include <QTextBrowser>
#include <QCloseEvent>
#include <QGraphicsScene>
#include "util.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

//OPENCV库引用
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#pragma comment(lib, "F:\\opencv\\build\\build-VC12-opencv249\\lib\\opencv_core249.lib")
#pragma comment(lib, "F:\\opencv\\build\\build-VC12-opencv249\\lib\\opencv_imgproc249.lib")
#pragma comment(lib, "F:\\opencv\\build\\build-VC12-opencv249\\lib\\opencv_highgui249.lib")
using namespace cv;
#endif



namespace Ui {
class ImageDisplay;
}

class ImageDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit ImageDisplay(QWidget *parent = 0);
    //窗口重绘
    //void paintEvent(QPaintEvent *);
    //图像调整尺寸
    void imageResize();
    //渲染图像
    void renderImage(QImage &image);
    //设置显示模式
    void setDisplayType(const QString &type, bool displayGray = false);
    //获取保存格式
    QString getSaveFormat();
    //保存当前图像
    void saveCurrentImage(const QString &savePath, bool autoMode = false);
    //改变保存格式
    void changeSaveFormat(const int id);
    //开关自动保存
    void changeAutoSaveFunc(const bool state, const QString &savePath);
    //改变自动保存路径
    void changeAutoMakeDirFunc(const bool state);
    //QImage转Mat
    cv::Mat QImageToMat(const QImage &image);
    //计算灰度值
    cv::Point getGrayBenchmark(cv::Mat img, int& grayValue, int& minValue, int& maxValue);

    ~ImageDisplay();

protected:
    //重写窗口改变大小事件
    void resizeEvent(QResizeEvent * event);
    //关闭窗口事件
    void closeEvent(QCloseEvent *event);
    //void keyPressEvent(QKeyEvent *event);

signals:
    //控制命令
    void controlImageDebugCMD(const QString &type, const QString &cmd);

private slots:
    //保存按键 单击槽函数
    void on_saveButton_clicked();
    //自动保存按键 单击槽函数
    void on_autosaveButton_clicked();
    //捕获按键 单击槽函数
    void on_captureButton_clicked();
    //自动创建文件夹复选框 单击槽函数
    void on_autoMakeDir_clicked(bool checked);
    //操作帮助按键 单击槽函数
    void on_pushButton_clicked();

private:
    Ui::ImageDisplay *ui;
    //配置文件
    QSettings *config;
    //自动保存路径
    QString autoSavePath;
    //图像类别
    QString imgType;
    //临时QImage图像
    QImage tempImg;
    //临时Mat图像
    Mat tempMat;
    //保存格式按钮组
    QButtonGroup *saveFormatRbGroup;
    //图形场景
    QGraphicsScene *scene;
    //缩放宽比例
    double scaleW = 0;
    //缩放高比例
    double scaleH = 0;
    //是否显示灰度
    bool displayGray;
    //自动保存
    bool autoSave = false;
    //自动创建文件夹
    bool autoMakeDirState = false;

};

#endif // IMAGEDISPLAY_H
