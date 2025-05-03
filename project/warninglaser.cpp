#include "warninglaser.h"
#include <QPen>
#include <QDebug>
#include <QLinearGradient>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

WarningLaser::WarningLaser(QGraphicsItem *parent) : QObject(nullptr), QGraphicsLineItem(parent) {
    QLinearGradient gradient(0, 0, 100, 0);
    gradient.setColorAt(0, QColor(0, 255, 0, 200)); // 纯绿色，更高不透明度
    gradient.setColorAt(0.5, QColor(50, 255, 50, 255)); // 中心亮绿，完全不透明
    gradient.setColorAt(1, QColor(0, 255, 0, 200)); // 纯绿色，更高不透明度
    QPen pen(QBrush(gradient), 4); // 增加线宽到 4 像素
    pen.setStyle(Qt::DashLine);
    pen.setCapStyle(Qt::RoundCap);
    pen.setDashPattern({4, 4});
    setPen(pen);

    // 添加发光效果
    QGraphicsDropShadowEffect *glow = new QGraphicsDropShadowEffect;
    glow->setColor(QColor(0, 255, 0, 150));
    glow->setBlurRadius(10);
    glow->setOffset(0, 0);
    setGraphicsEffect(glow);

    countdownTimer = new QTimer(this);
    countdownTimer->setInterval(1000);
    remainingTime = 0;

    QPropertyAnimation *blink = new QPropertyAnimation(this, "opacity");
    blink->setDuration(500);
    blink->setKeyValueAt(0, 1.0);
    blink->setKeyValueAt(0.5, 0.7); // 最低透明度 0.7
    blink->setKeyValueAt(1, 1.0);
    blink->setLoopCount(-1);
    blink->start(QAbstractAnimation::DeleteWhenStopped);

    QObject::connect(countdownTimer, &QTimer::timeout, [this, blink]() {
        remainingTime--;
        qDebug() << "WarningLaser countdown:" << remainingTime;
        if (remainingTime <= 0) {
            countdownTimer->stop();
            blink->stop();
            emit countdownFinished();
            qDebug() << "Countdown finished, emitting signal";
        }
    });
}

WarningLaser::~WarningLaser() {
    countdownTimer->stop();
    delete countdownTimer;
    qDebug() << "WarningLaser destroyed";
}

void WarningLaser::setCountdown(int seconds) {
    remainingTime = seconds;
    countdownTimer->start();
}

void WarningLaser::stopCountdown() {
    if (countdownTimer->isActive()) {
        countdownTimer->stop();
        qDebug() << "WarningLaser countdown timer stopped";
    }
}
