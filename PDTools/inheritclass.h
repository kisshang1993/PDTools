/*************************************************
Copyright: 2018-2019 Melux
Author: ChengHang
Date:2019-01-17
Description: 重写一些常用的类来抛出一些事件与函数
**************************************************/
#ifndef INHERITCLASS_H
#define INHERITCLASS_H
#include <QWidget>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QStyleOption>
#include <QPainter>
#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QPoint>
#include <QDebug>
#include "util.h"

//QWidget继承抛出事件
class EventQWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EventQWidget(QWidget *parent = 0);


signals:
    void close();
    void pressEsc();

protected:
    //关闭
    void closeEvent(QCloseEvent *event);
    //按键
    void keyPressEvent(QKeyEvent *event);

};


//QWidget继承抛出事件
class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);
    void setTitle(const QString title);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    bool dragMode = false;
    QPoint cursorPoint;
    QLabel *titleLabel;

};



#endif // INHERITCLASS_H
