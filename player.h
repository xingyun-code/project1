#ifndef PLAYER_H
#define PLAYER_H
#include <QGraphicsEllipseItem>
#include <QKeyEvent>
#include <QTimer>

class Player : public QGraphicsEllipseItem {
public:
    Player(QGraphicsItem *parent = nullptr);
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override; // 处理按键释放
    void setCircleBounds(int centerX, int centerY, int radius);

    QTimer *moveTimer;  // 定时器
    int dx = 0;
    int dy = 0;
private:
    int circleCenterX;
    int circleCenterY;
    int circleRadius;
};
#endif
