#include <QWheelEvent>
#include <QKeyEvent>
#include "interactive_view.h"

#define VIEW_CENTER viewport()->rect().center()
#define VIEW_WIDTH  viewport()->rect().width()
#define VIEW_HEIGHT viewport()->rect().height()

InteractiveView::InteractiveView(QWidget *parent)
    : QGraphicsView(parent),
      m_translateButton(Qt::LeftButton),
      m_scale(1.0),
      m_zoomDelta(0.1),
      m_translateSpeed(1.0),
      m_bMouseTranslate(false)
{
    // 去掉滚动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCursor(Qt::PointingHandCursor);
    setRenderHint(QPainter::Antialiasing);

    setSceneRect(INT_MIN/2, INT_MIN/2, INT_MAX, INT_MAX);
    centerOn(0, 0);

}

/**
 * @brief 设置平移速度
 * @param speed
 */
void InteractiveView::setTranslateSpeed(qreal speed)
{
    // 建议速度范围
    Q_ASSERT_X(speed >= 0.0 && speed <= 2.0,
               "InteractiveView::setTranslateSpeed", "Speed should be in range [0.0, 2.0].");
    m_translateSpeed = speed;
}

/**
 * @brief 获取平移速度
 * @return speed
 */
qreal InteractiveView::getTranslateSpeed() const
{
    return m_translateSpeed;
}

/**
 * @brief 设置缩放增量
 * @param delta
 */
void InteractiveView::setZoomDelta(qreal delta)
{
    // 建议增量范围
    Q_ASSERT_X(delta >= 0.0 && delta <= 1.0,
               "InteractiveView::setZoomDelta", "Delta should be in range [0.0, 1.0].");
    m_zoomDelta = delta;
}

/**
 * @brief 获取缩放增量
 * @return 缩放增量
 */
qreal InteractiveView::getZoomDelta() const
{
    return m_zoomDelta;
}

/**
 * @brief 控制移动
 * @param event
 * @note 上/下/左/右键向各个方向移动、
 *       加/减键进行缩放、空格/回车键旋转
 */
void InteractiveView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        translate(QPointF(0, -2));  // 上移
        break;
    case Qt::Key_Down:
        translate(QPointF(0, 2));  // 下移
        break;
    case Qt::Key_Left:
        translate(QPointF(-2, 0));  // 左移
        break;
    case Qt::Key_Right:
        translate(QPointF(2, 0));  // 右移
        break;
    case Qt::Key_W:  // 放大
        zoomIn();
        break;
    case Qt::Key_S:  // 缩小
        zoomOut();
        break;
    case Qt::Key_A:  // 逆时针旋转
        rotate(-5);
        break;
    case Qt::Key_D:  // 顺时针旋转
        rotate(5);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}

/**
 * @brief 平移 移动
 * @param event
 */
void InteractiveView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bMouseTranslate){
        QPointF mouseDelta = mapToScene(event->pos()) - mapToScene(m_lastMousePos);
        translate(mouseDelta);
    }

    m_lastMousePos = event->pos();

    QGraphicsView::mouseMoveEvent(event);
}

/**
 * @brief 平移 按下
 * @param event
 */
void InteractiveView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == m_translateButton) {
        // 当光标底下没有 item 时，才能移动
        QPointF point = mapToScene(event->pos());
//        if (scene()->itemAt(point, transform()) == NULL)  {
//            m_bMouseTranslate = true;
//            m_lastMousePos = event->pos();
//        }
        m_bMouseTranslate = true;
        m_lastMousePos = event->pos();

    }

    QGraphicsView::mousePressEvent(event);
}

/**
 * @brief 平移 松开
 * @param event
 */
void InteractiveView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == m_translateButton)
        m_bMouseTranslate = false;

    QGraphicsView::mouseReleaseEvent(event);
}

/**
 * @brief 滚轮事件 放大/缩小
 * @param event
 */
void InteractiveView::wheelEvent(QWheelEvent *event)
{
    // 滚轮的滚动量
    QPoint scrollAmount = event->angleDelta();
    // 正值表示滚轮远离使用者（放大），负值表示朝向使用者（缩小）
    scrollAmount.y() > 0 ? zoomIn() : zoomOut();
}

/**
 * @brief 放大
 */
void InteractiveView::zoomIn()
{
    zoom(1 + m_zoomDelta);
}

/**
 * @brief 缩小
 */
void InteractiveView::zoomOut()
{
    zoom(1 - m_zoomDelta);
}

/**
 * @brief 缩放
 * @param scaleFactor
 */
void InteractiveView::zoom(float scaleFactor)
{
    // 防止过小或过大
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    scale(scaleFactor, scaleFactor);
    m_scale *= scaleFactor;
}

/**
 * @brief 平移
 * @param delta
 */
void InteractiveView::translate(QPointF delta)
{
    // 根据当前 zoom 缩放平移数
    delta *= m_scale;
    delta *= m_translateSpeed;

    // view 根据鼠标下的点作为锚点来定位 scene
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    QPoint newCenter(VIEW_WIDTH / 2 - delta.x(),  VIEW_HEIGHT / 2 - delta.y());
    centerOn(mapToScene(newCenter));

    // scene 在 view 的中心点作为锚点
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
}
