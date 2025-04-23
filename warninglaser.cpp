#include "warninglaser.h"
#include <QPen>
#include <QGraphicsScene>

// 显式调用 QObject 的构造函数
WarningLaser::WarningLaser(QGraphicsItem *parent) : QObject(nullptr), QGraphicsLineItem(parent) {
    QPen pen(Qt::red, 5); // 线宽改为 5
    pen.setStyle(Qt::DashLine); // 虚线样式
    setPen(pen);
    remainingTime = 3; // 默认倒计时 3 秒
    countdownTimer = new QTimer(this); // 在构造函数中创建定时器
}

void WarningLaser::setCountdown(int seconds) {
    remainingTime = seconds;
    // 启动倒计时定时器
    QObject::connect(countdownTimer, &QTimer::timeout, [this]() {
        remainingTime--;
        if (remainingTime <= 0) {
            countdownTimer->stop();
            emit countdownFinished(); // 发射信号（需要 Q_OBJECT 宏）
            scene()->removeItem(this);
            delete this;
        }
    });
    countdownTimer->start(1000); // 每秒触发
}
