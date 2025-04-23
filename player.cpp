#include "player.h"
#include <QBrush>
#include <QTimer>
#include <QObject>
#include <qmath.h>

Player::Player(QGraphicsItem *parent) : QGraphicsEllipseItem(parent) {
    int diameter = 30;
    setRect(0, 0, diameter, diameter);
    setPos(400 - diameter/2, 300 - diameter/2);
    setBrush(Qt::blue);
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();

    moveTimer = new QTimer;
    QObject::connect(moveTimer, &QTimer::timeout, [this]() {
        int newX = pos().x() + dx * 10;
        int newY = pos().y() + dy * 10;
        int distance = qSqrt(qPow(newX - circleCenterX, 2) + qPow(newY - circleCenterY, 2));
        if (distance <= circleRadius) {
            moveBy(dx * 10, dy * 10);
        }
    });
}

void Player::setCircleBounds(int centerX, int centerY, int radius) {
    circleCenterX = centerX;
    circleCenterY = centerY;
    circleRadius = radius;
}

// 按键按下事件
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

// 按键释放事件
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
