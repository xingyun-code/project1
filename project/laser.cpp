#include "laser.h"
#include <QPen>
#include <QGraphicsScene>
#include <QDebug>
#include <QLinearGradient>
#include <QPropertyAnimation>

Laser::Laser(QGraphicsItem *parent) : QObject(nullptr), QGraphicsLineItem(parent) {
    // 初始渐变，与 setLine 一致
    QLinearGradient gradient(0, 0, 100, 0);
    gradient.setColorAt(0, QColor(255, 0, 0, 255)); // 起点：高亮红色
    gradient.setColorAt(0.7, QColor(255, 0, 0, 255)); // 中间偏后：高亮红色
    gradient.setColorAt(1, QColor(255, 0, 0, 0));   // 尾部：透明
    QPen pen(QBrush(gradient), 3);
    pen.setCapStyle(Qt::RoundCap);
    setPen(pen);

    moveTimer = new QTimer(this);
    lifetimeTimer = new QTimer(this);
    lifetimeTimer->setSingleShot(true);
    velocityX = 0;
    velocityY = 0;

    QPropertyAnimation *fadeIn = new QPropertyAnimation(this, "opacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

    QObject::connect(lifetimeTimer, &QTimer::timeout, [this]() {
        QPropertyAnimation *fadeOut = new QPropertyAnimation(this, "opacity");
        fadeOut->setDuration(300);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        QObject::connect(fadeOut, &QPropertyAnimation::finished, [this]() {
            emit lifetimeFinished(this);
        });
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

Laser::~Laser() {
    if (moveTimer) {
        moveTimer->stop();
        moveTimer->deleteLater();
    }
    if (lifetimeTimer) {
        lifetimeTimer->stop();
        lifetimeTimer->deleteLater();
    }
    qDebug() << "Laser destroyed";
}

void Laser::setLine(qreal x1, qreal y1, qreal x2, qreal y2) {
    QGraphicsLineItem::setLine(x1, y1, x2, y2);
    // 动态调整渐变长度，强调尾部渐变
    QLinearGradient gradient(x1, y1, x2, y2);
    gradient.setColorAt(0, QColor(255, 0, 0, 255)); // 起点：高亮红色
    gradient.setColorAt(0.7, QColor(255, 0, 0, 255)); // 中间偏后：高亮红色
    gradient.setColorAt(1, QColor(255, 0, 0, 0));   // 尾部：透明
    QPen pen = this->pen();
    pen.setBrush(QBrush(gradient));
    setPen(pen);
}

void Laser::setVelocity(double vx, double vy) {
    velocityX = vx;
    velocityY = vy;
}

void Laser::startLifetime(int msec) {
    lifetimeTimer->start(msec);
}

void Laser::stopLifetime() {
    if (lifetimeTimer->isActive()) {
        lifetimeTimer->stop();
        qDebug() << "Laser lifetime timer stopped";
    }
}
