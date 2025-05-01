#ifndef WARNINGLASER_H
#define WARNINGLASER_H

#include <QGraphicsLineItem>
#include <QTimer>

class WarningLaser : public QObject, public QGraphicsLineItem {
    Q_OBJECT
public:
    WarningLaser(QGraphicsItem *parent = nullptr);
    ~WarningLaser();

    void setCountdown(int seconds);
    void stopCountdown();

    int remainingTime;
    QTimer *countdownTimer;

signals:
    void countdownFinished();
};

#endif // WARNINGLASER_H
