#ifndef LASER_H
#define LASER_H
#include <QGraphicsLineItem>
#include <QTimer>

class Laser : public QObject, public QGraphicsLineItem {
    Q_OBJECT
public:
    Laser(QGraphicsItem *parent = nullptr);
    ~Laser();
    void setLine(qreal x1, qreal y1, qreal x2, qreal y2);
    void setVelocity(double vx, double vy);
    void startLifetime(int msec);
    void stopLifetime();

    double velocityX, velocityY;
    QTimer *moveTimer;

signals:
    void lifetimeFinished(Laser *laser);

private:
    QTimer *lifetimeTimer;
};
#endif
