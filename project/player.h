#ifndef PLAYER_H
#define PLAYER_H
#include <QGraphicsRectItem>
#include <QKeyEvent>
#include <QTimer>
#include <qtimer.h>
#include <QObject>


class Player : public QGraphicsRectItem {
public:
    Player(QGraphicsItem *parent = nullptr);
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override; // 处理按键释放

private:
    QTimer *moveTimer;  // 定时器驱动移动
    int dx = 0;         // 水平方向（-1: 左, 1: 右）
    int dy = 0;         // 垂直方向（-1: 上, 1: 下）
};
#endif
