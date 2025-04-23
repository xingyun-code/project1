#ifndef LASER_H
#define LASER_H
#include <QGraphicsLineItem>
#include <QTimer>

class Laser : public QObject, public QGraphicsLineItem {
public:
    Laser(QGraphicsItem *parent = nullptr);
    void setVelocity(double vx, double vy); // 修改参数类型为 double
    double velocityX, velocityY; // 修改成员变量类型为 double
    QTimer *moveTimer;
    ~Laser();
};
#endif
