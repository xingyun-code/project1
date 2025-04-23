#include "laser.h"
#include <QPen>
#include <QGraphicsScene>

Laser::Laser(QGraphicsItem *parent) : QObject(nullptr), QGraphicsLineItem(parent) {
    QPen pen(Qt::red, 5); // 线宽改为 5
    setPen(pen);
}

Laser::~Laser() {
    // 不需要删除定时器，因为使用了 QTimer::singleShot
}
