#include "warninglaser.h"
#include <QPen>
#include <QDebug>

WarningLaser::WarningLaser(QGraphicsItem *parent) : QObject(nullptr), QGraphicsLineItem(parent) {
    QPen pen(Qt::green, 5);
    pen.setStyle(Qt::DashLine);
    setPen(pen);
    countdownTimer = new QTimer(this);
    countdownTimer->setInterval(1000);
    remainingTime = 0;
    QObject::connect(countdownTimer, &QTimer::timeout, [this]() {
        remainingTime--;
        qDebug() << "WarningLaser countdown:" << remainingTime;
        if (remainingTime <= 0) {
            countdownTimer->stop();
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
