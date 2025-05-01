#ifndef PLAYER_H
#define PLAYER_H
#include <QGraphicsEllipseItem>
#include <QKeyEvent>
#include <QTimer>

class Player : public QGraphicsEllipseItem {
public:
    Player(QGraphicsItem *parent = nullptr);
    ~Player();
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void setCircleBounds(int centerX, int centerY, int radius);
    void resetState();

    QTimer *moveTimer;
    int dx = 0;
    int dy = 0;
private:
    int circleCenterX;
    int circleCenterY;
    int circleRadius;
};

#endif
