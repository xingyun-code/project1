#include "player.h"
#include <QBrush>
#include <QTimer>
#include <QObject>
#include "qobject.h"

Player::Player(QGraphicsItem *parent) : QGraphicsRectItem(parent) {
    setRect(0, 0, 30, 30);
    setBrush(Qt::blue);
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();

    // 初始化定时器（每 30ms 触发一次移动）
    moveTimer = new QTimer();
    QInputDevice::connect(moveTimer, &QTimer::timeout, [this]() {
        moveBy(dx * 5, dy * 5);
    });
    moveTimer->start(30);

    QInputDevice::connect(moveTimer, &QTimer::timeout, [this]() {
        moveBy(dx * 5, dy * 5); // 步长为 5 像素
    });
}

// 按键按下事件：记录方向
void Player::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Left:  dx = -1; break;
    case Qt::Key_Right: dx = 1;  break;
    case Qt::Key_Up:    dy = -1; break;
    case Qt::Key_Down:  dy = 1; break;
    default: return;
    }
    if (!moveTimer->isActive()) {
        moveTimer->start(30); // 启动定时器
    }
}

// 按键释放事件：清除方向
void Player::keyReleaseEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right: dx = 0; break;
    case Qt::Key_Up:
    case Qt::Key_Down:  dy = 0; break;
    default: return;
    }
    if (dx == 0 && dy == 0) {
        moveTimer->stop(); // 停止定时器
    }
}
