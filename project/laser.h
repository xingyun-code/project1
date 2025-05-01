#ifndef LASER_H
#define LASER_H
#include <QGraphicsLineItem>
#include <QTimer>

class Laser : public QObject, public QGraphicsLineItem {
public:
    Laser(QGraphicsItem *parent = nullptr);
    void setVelocity(double vx, double vy);
    double velocityX, velocityY;
    QTimer *moveTimer;
    ~Laser();
};
#endif
