/*************************************************
Copyright: 2018-2019 Melux
Author: ChengHang
Date:2019-01-17
Description: 重写一些常用的类来抛出一些事件与函数
**************************************************/
#include "inheritclass.h"


/**
 * @brief EventQWidget::EventQWidget
 * @param parent
 */
EventQWidget::EventQWidget(QWidget *parent){}

/**
 * @brief 响应关闭，发送信号
 * @param event
 */
void EventQWidget::closeEvent(QCloseEvent *event)
{
    emit close();
}

/**
 * @brief 响应键盘事件
 * @param event
 */
void EventQWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        emit pressEsc();
    }
}

/**
 * @brief 标题栏构造函数
 * @param parent
 */
TitleBar::TitleBar(QWidget *parent) : QWidget(parent)
{
    parent->setWindowFlags(Qt::FramelessWindowHint);
    parent->setAttribute(Qt::WA_TranslucentBackground, true);
    int barHeight = 30;
    this->setStyleSheet("background: #FFF");
    this->setGeometry(5, 5, parent->width()-10, barHeight);
    //横向布局
    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(10, 0, 5, 0);
    //图标
    QLabel *icon = new QLabel(this);
    icon->setGeometry(0, 0, 15, 15);
    icon->setPixmap(QPixmap::fromImage(Util::ScaleImage2Label(QImage(":/pixmap/images/favicon.ico"), icon)));
    hbox->addWidget(icon);
    hbox->addSpacing(5);

    //标题
    titleLabel = new QLabel(this);
    titleLabel->setText("PDTools");
    titleLabel->setStyleSheet("font-size:16px;font-family:'microsoft yahei';");
    hbox->addWidget(titleLabel);

    //关闭按钮
    QToolButton *closeButton= new QToolButton(this);
    //QPixmap closePix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
    QPixmap closePix = QPixmap(":/pixmap/images/close.png");
    closeButton->setIcon(closePix);
    closeButton->setStyleSheet("background-color:transparent;");
    closeButton->setGeometry(0,0, 15, 15);
    connect(closeButton, &QToolButton::clicked, [=](){
       parent->close();
    });

    hbox->addStretch();
    hbox->addWidget(closeButton);
    this->setLayout(hbox);
    this->show();
}

/**
 * @brief 窗口重绘
 */
void TitleBar::paintEvent(QPaintEvent *)
{
    //重绘样式
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

/**
 * @brief 设置标题
 * @param title
 */
void TitleBar::setTitle(const QString title)
{
    titleLabel->setText(title);
}

/**
 * @brief 鼠标点击事件
 * @param event
 */
void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if(event->button() ==Qt::LeftButton)
    {
        dragMode = true;
        cursorPoint = event->pos();
    }
}

/**
 * @brief 鼠标释放事件
 * @param event
 */
void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    dragMode = false;
}

/**
 * @brief 鼠标移动事件
 * @param event
 */
void TitleBar::mouseMoveEvent(QMouseEvent *event)
{

    if(dragMode)
    {
        this->parentWidget()->move(event->globalPos().x() - cursorPoint.x(),
                   event->globalPos().y() - cursorPoint.y());
    }
}

