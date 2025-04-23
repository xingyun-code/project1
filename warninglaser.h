#ifndef WARNINGLASER_H
#define WARNINGLASER_H
#include <QGraphicsLineItem>
#include <QTimer>
#include <QObject>

// 同时继承 QObject 和 QGraphicsLineItem
class WarningLaser : public QObject, public QGraphicsLineItem {
    Q_OBJECT
public:
    WarningLaser(QGraphicsItem *parent = nullptr);
    void setCountdown(int seconds);// 设置倒计时
    QTimer *countdownTimer;
    int remainingTime;
signals:
    void countdownFinished();
};
#endif
