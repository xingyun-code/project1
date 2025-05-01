#include "laser.h"
#include <QPen>
#include <QGraphicsScene>
#include <QDebug>

Laser::Laser(QGraphicsItem *parent) : QObject(nullptr), QGraphicsLineItem(parent) {
    QPen pen(Qt::red, 5);
    setPen(pen);
    moveTimer = new QTimer(this);
    velocityX = 0;
    velocityY = 0;
}

Laser::~Laser() {
    if (moveTimer) {
        moveTimer->stop();
        moveTimer->deleteLater();
        qDebug() << "Laser destroyed";
    }
}

void Laser::setVelocity(double vx, double vy) {
    velocityX = vx;
    velocityY = vy;
}
