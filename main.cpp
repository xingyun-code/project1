#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QTimer>
#include <QRandomGenerator>
#include <QGraphicsEllipseItem>
#include "player.h"
#include "warninglaser.h"
#include "laser.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    int size = 800;
    view.setFixedSize(size, size);
    scene.setSceneRect(0, 0, size, size);

    // 绘制大圆
    int circleRadius = 350;
    int circleCenterX = size / 2;
    int circleCenterY = size / 2;
    QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(circleCenterX - circleRadius, circleCenterY - circleRadius, 2 * circleRadius, 2 * circleRadius);
    circle->setPen(QPen(Qt::black, 2));
    scene.addItem(circle);

    // 声明倒计时标签
    QLabel *countdownLabel = new QLabel(&view);
    countdownLabel->setStyleSheet("font: 24px; color: white;");
    countdownLabel->move(700, 10);
    countdownLabel->show();

    // 生成提示激光的定时器
    QTimer *spawnTimer = new QTimer();
    QObject::connect(spawnTimer, &QTimer::timeout, [&]() {
        // 随机生成激光条数（5到10）
        int laserCount = QRandomGenerator::global()->bounded(5, 11); // [5, 10]

        // 计算均匀角度间隔
        qreal angleStep = 360.0 / laserCount;

        for (int i = 0; i < laserCount; ++i) {
            // 均匀分布起点角度
            qreal startAngle = i * angleStep;
            qreal startRad = qDegreesToRadians(startAngle);

            // 计算激光的起点（落在圆周上）
            qreal startX = circleCenterX + circleRadius * qCos(startRad);
            qreal startY = circleCenterY + circleRadius * qSin(startRad);

            // 计算对侧点角度（直径方向）
            qreal oppositeAngle = startAngle + 180;
            oppositeAngle = fmod(oppositeAngle, 360);
            if (oppositeAngle < 0) oppositeAngle += 360;

            // 生成方向角度（与直径方向夹角小于45度）
            qreal minAngle = oppositeAngle - 45;
            qreal maxAngle = oppositeAngle + 45;
            qreal directionAngle = minAngle + (maxAngle - minAngle) * QRandomGenerator::global()->generateDouble();
            // 规范化方向角度到 [0, 360)
            directionAngle = fmod(directionAngle, 360);
            if (directionAngle < 0) directionAngle += 360;
            qreal directionRad = qDegreesToRadians(directionAngle);

            // 计算方向向量
            qreal dx = qCos(directionRad);
            qreal dy = qSin(directionRad);

            // 计算实体激光的终点（使用方向角度，确保在圆周上）
            qreal endX = circleCenterX + circleRadius * dx;
            qreal endY = circleCenterY + circleRadius * dy;

            // 计算提示激光的长度（1/8 个直径）
            qreal warningLength = circleRadius * 2 / 8;

            // 提示激光与实体激光同方向
            qreal warningEndX = startX + warningLength * dx;
            qreal warningEndY = startY + warningLength * dy;

            // 验证提示激光终点在圆内
            qreal warningEndDistance = qSqrt(qPow(warningEndX - circleCenterX, 2) + qPow(warningEndY - circleCenterY, 2));
            if (warningEndDistance > circleRadius) {
                qreal warningRatio = circleRadius / warningEndDistance;
                warningEndX = circleCenterX + (warningEndX - circleCenterX) * warningRatio;
                warningEndY = circleCenterY + (warningEndY - circleCenterY) * warningRatio;
            }

            // 生成提示激光
            WarningLaser *warning = new WarningLaser();
            warning->setLine(startX, startY, warningEndX, warningEndY);
            scene.addItem(warning);
            warning->setCountdown(3); // 3 秒倒计时

            // 更新倒计时标签（仅为最后一条激光更新，避免闪烁）
            if (i == laserCount - 1) {
                countdownLabel->setText("倒计时: " + QString::number(warning->remainingTime));
                QObject::connect(warning->countdownTimer, &QTimer::timeout, [=]() {
                    countdownLabel->setText("倒计时: " + QString::number(warning->remainingTime));
                });
            }

            // 连接倒计时结束信号到创建激光的槽函数
            QObject::connect(warning, &WarningLaser::countdownFinished, [&, startX, startY, endX, endY]() {
                // 生成实体激光
                Laser *laser = new Laser();
                laser->setLine(startX, startY, endX, endY);
                scene.addItem(laser);
                // 实体激光停留一段时间后消失
                QTimer::singleShot(2000, [laser]() {
                    if (laser->scene()) {
                        laser->scene()->removeItem(laser);
                    }
                    delete laser;
                });
            });
        }
    });
    spawnTimer->start(5000); // 每 5 秒生成一次

    Player *player = new Player();
    player->setCircleBounds(circleCenterX, circleCenterY, circleRadius);
    scene.addItem(player);
    view.show();

    return a.exec();
}
