#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QTimer>
#include <QRandomGenerator>
#include <QGraphicsEllipseItem>
#include <QDebug>
#include <cmath>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGraphicsPixmapItem>
#include "player.h"
#include "warninglaser.h"
#include "laser.h"
#include <QFile>
#include <QDir>

class GameView : public QGraphicsView {
public:
    GameView(QGraphicsScene *scene, QWidget *parent = nullptr) : QGraphicsView(scene, parent) {
        setFocusPolicy(Qt::StrongFocus);
        gameOver = nullptr;
        spawnTimer = nullptr;
        collisionTimer = nullptr;
        player = nullptr;
        score = nullptr;
        currentWaveLasers = nullptr;
        scoreLabel = nullptr;
        gameOverLabel = nullptr;
        scenePtr = nullptr;
        circleCenterX = 0;
        circleCenterY = 0;
        circleRadius = 0;
        countdownLabel = nullptr;
        circleItem = nullptr;
        backgroundItem = nullptr;
        quitRequested = false;
        currentWaveLaserCount = 0;
        remainingLasersInWave = 0;
        isFirstWave = true;
        collidingLaser = nullptr;
        qDebug() << "GameView constructed";
    }

    ~GameView() {
        qDebug() << "GameView destroyed";
    }

    void setBackgroundItem(QGraphicsPixmapItem *bgItem) {
        backgroundItem = bgItem;
    }

    void setGameState(bool *over, QTimer *sTimer, QTimer *cTimer, Player *p, int *scorePtr, int *c, QLabel *sLabel, QLabel *gLabel, QGraphicsScene *scene, int cX, int cY, int r, QLabel *cdLabel, QGraphicsEllipseItem *circle) {
        gameOver = over;
        spawnTimer = sTimer;
        collisionTimer = cTimer;
        player = p;
        score = scorePtr;
        currentWaveLasers = c;
        scoreLabel = sLabel;
        gameOverLabel = gLabel;
        scenePtr = scene;
        circleCenterX = cX;
        circleCenterY = cY;
        circleRadius = r;
        countdownLabel = cdLabel;
        circleItem = circle;
        qDebug() << "Game state set";
    }

    void setupSpawnTimer() {
        spawnTimer->disconnect();
        QObject::connect(spawnTimer, &QTimer::timeout, [this]() {
            if (*gameOver || !scenePtr) {
                qDebug() << "Spawn timer triggered but game is over or scenePtr is invalid";
                return;
            }

            qDebug() << "Spawn timer triggered, creating lasers";
            int laserCount = QRandomGenerator::global()->bounded(5, 11);
            *currentWaveLasers = laserCount;
            currentWaveLaserCount = laserCount;
            remainingLasersInWave = laserCount;
            qreal angleStep = 360.0 / laserCount;

            for (int i = 0; i < laserCount; ++i) {
                // 计算激光起点，严格位于圆周上
                qreal startAngle = i * angleStep;
                qreal startRad = qDegreesToRadians(startAngle);
                qreal startX = circleCenterX + circleRadius * std::cos(startRad);
                qreal startY = circleCenterY + circleRadius * std::sin(startRad);
                // 四舍五入到整数像素以对齐网格，防止偏移
                startX = std::round(startX);
                startY = std::round(startY);

                // 计算激光方向（指向圆心附近，±45°范围内）
                qreal oppositeAngle = startAngle + 180;
                oppositeAngle = std::fmod(oppositeAngle, 360.0);
                if (oppositeAngle < 0) oppositeAngle += 360.0;

                qreal minAngle = oppositeAngle - 45;
                qreal maxAngle = oppositeAngle + 45;
                qreal directionAngle = minAngle + (maxAngle - minAngle) * QRandomGenerator::global()->generateDouble();
                directionAngle = std::fmod(directionAngle, 360.0);
                if (directionAngle < 0) directionAngle += 360.0;
                qreal directionRad = qDegreesToRadians(directionAngle);

                qreal dx = std::cos(directionRad);
                qreal dy = std::sin(directionRad);
                qreal length = std::sqrt(dx * dx + dy * dy);
                if (length != 0) {
                    dx /= length;
                    dy /= length;
                }

                // 计算红色激光终点：激光线与圆周的交点（对侧）
                // 线方程：x = startX + t * dx, y = startY + t * dy
                // 圆方程：(x - 400)^2 + (y - 400)^2 = 350^2
                qreal cx = circleCenterX; // 400
                qreal cy = circleCenterY; // 400
                qreal r = circleRadius;   // 350
                qreal a = dx * dx + dy * dy; // 应为 1（dx, dy 已归一化）
                qreal b = 2 * (dx * (startX - cx) + dy * (startY - cy));
                qreal c = (startX - cx) * (startX - cx) + (startY - cy) * (startY - cy) - r * r;
                qreal discriminant = b * b - 4 * a * c;

                if (discriminant < 0) {
                    qDebug() << "Error: No intersection with circle for laser" << i;
                    continue; // 跳过此激光
                }

                qreal t1 = (-b + std::sqrt(discriminant)) / (2 * a);
                qreal t2 = (-b - std::sqrt(discriminant)) / (2 * a);
                // 选择远离起点的交点（较大的 t，通常为 t1）
                qreal t = (t1 > t2 && t1 > 0) ? t1 : t2;
                if (t <= 0) {
                    qDebug() << "Warning: Invalid intersection t =" << t << "for laser" << i;
                    t = t1; // 回退到 t1
                }

                qreal endX = startX + t * dx;
                qreal endY = startY + t * dy;
                // 四舍五入终点坐标
                endX = std::round(endX);
                endY = std::round(endY);

                // 验证终点是否在圆周上
                qreal distToCenter = std::sqrt((endX - cx) * (endX - cx) + (endY - cy) * (endY - cy));
                if (std::abs(distToCenter - r) > 1.0) {
                    qDebug() << "Warning: End point (" << endX << "," << endY << ") not on circle, distance =" << distToCenter;
                }

                // 绿色警告激光长度：1/8 圆直径
                qreal warningLength = (2 * circleRadius) / 8; // 87.5 像素
                qreal warningEndX = startX + warningLength * dx;
                qreal warningEndY = startY + warningLength * dy;
                // 四舍五入警告激光坐标
                warningEndX = std::round(warningEndX);
                warningEndY = std::round(warningEndY);

                WarningLaser *warning = new WarningLaser();
                warning->setLine(startX, startY, warningEndX, warningEndY);
                if (scenePtr) {
                    scenePtr->addItem(warning);
                } else {
                    qDebug() << "Error: scenePtr is null, cannot add WarningLaser";
                    delete warning;
                    continue;
                }
                warning->setCountdown(3);
                qDebug() << "WarningLaser" << i << "created at (" << startX << "," << startY << ") to (" << warningEndX << "," << warningEndY << ") with direction (" << dx << "," << dy << ")";

                if (i == laserCount - 1) {
                    countdownLabel->setText("倒计时: " + QString::number(warning->remainingTime));
                    countdownLabel->adjustSize();
                    QObject::connect(warning->countdownTimer, &QTimer::timeout, [this, warning]() {
                        countdownLabel->setText("倒计时: " + QString::number(warning->remainingTime));
                        countdownLabel->adjustSize();
                        qDebug() << "Countdown updated:" << warning->remainingTime;
                    });
                }

                QObject::connect(warning, &WarningLaser::countdownFinished, [startX, startY, endX, endY, this, warning]() {
                    if (*gameOver || !scenePtr) {
                        qDebug() << "Countdown finished but game is over or scenePtr is invalid";
                        return;
                    }

                    Laser *laser = new Laser();
                    laser->setLine(startX, startY, endX, endY);
                    scenePtr->addItem(laser);
                    qDebug() << "Laser created at (" << startX << "," << startY << ") to (" << endX << "," << endY << ")";

                    laser->startLifetime(1500); // 缩短生命周期到 1500ms

                    QObject::connect(laser, &Laser::lifetimeFinished, [this](QObject *obj) {
                        Laser *laser = qobject_cast<Laser*>(obj);
                        if (!laser) return;
                        if (laser->scene()) {
                            laser->scene()->removeItem(laser);
                        }
                        delete laser;
                        qDebug() << "Laser deleted via lifetimeFinished";

                        (*currentWaveLasers)--;
                        remainingLasersInWave--;
                        qDebug() << "Remaining lasers in wave:" << remainingLasersInWave;

                        if (remainingLasersInWave == 0 && !*gameOver) {
                            (*score)++;
                            scoreLabel->setText("得分: " + QString::number(*score));
                            scoreLabel->adjustSize();
                            qDebug() << "Score updated:" << *score;
                        }
                    });

                    if (warning->scene()) {
                        scenePtr->removeItem(warning);
                    }
                    delete warning;
                    qDebug() << "WarningLaser deleted";
                });
            }

            if (isFirstWave) {
                isFirstWave = false;
                spawnTimer->setInterval(5000);
                qDebug() << "First wave spawned, setting subsequent spawn interval to 5000ms";
            }
        });
        qDebug() << "Spawn timer set up";
    }

    void setupCollisionTimer() {
        collisionTimer->disconnect();
        QObject::connect(collisionTimer, &QTimer::timeout, [this]() {
            if (*gameOver || !scenePtr || !player) {
                collisionTimer->stop();
                qDebug() << "Collision timer triggered but game is over or scenePtr/player is invalid";
                return;
            }

            qDebug() << "Collision timer triggered, checking collisions";
            QList<QGraphicsItem *> items = scenePtr->items();
            for (QGraphicsItem *item : items) {
                Laser *laser = dynamic_cast<Laser *>(item);
                if (laser && player && player->collidesWithItem(laser)) {
                    *gameOver = true;
                    gameOverLabel->show();
                    spawnTimer->stop();
                    collisionTimer->stop();

                    // 停止所有激光的生命周期定时器，以保留它们
                    for (QGraphicsItem *sceneItem : scenePtr->items()) {
                        if (Laser *activeLaser = dynamic_cast<Laser *>(sceneItem)) {
                            activeLaser->stopLifetime();
                            qDebug() << "Laser lifetime timer stopped to retain it";
                        }
                        if (WarningLaser *warning = dynamic_cast<WarningLaser *>(sceneItem)) {
                            warning->stopCountdown();
                            scenePtr->removeItem(warning);
                            delete warning;
                            qDebug() << "WarningLaser deleted during Game Over cleanup";
                        }
                    }

                    items = scenePtr->items();
                    qDebug() << "Items remaining in scene after Game Over cleanup:" << items.size();
                    for (QGraphicsItem *item : items) {
                        qDebug() << "Remaining item type:" << typeid(*item).name();
                    }

                    qDebug() << "Game Over: Player collided with laser, all lasers retained";
                    break;
                }
            }
        });
        qDebug() << "Collision timer set up";
    }

protected:
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_R && gameOver && *gameOver) {
            qDebug() << "Restart initiated";
            *gameOver = false;
            *score = 0;
            *currentWaveLasers = 0;
            currentWaveLaserCount = 0;
            remainingLasersInWave = 0;
            isFirstWave = true;
            scoreLabel->setText("得分: " + QString::number(*score));
            scoreLabel->adjustSize();
            gameOverLabel->hide();
            qDebug() << "Game state reset: gameOver =" << *gameOver << ", score =" << *score;

            if (player) {
                player->setPos(circleCenterX - 15, circleCenterY - 15);
                player->resetState();
                qDebug() << "Player position reset to (" << circleCenterX - 15 << "," << circleCenterY - 15 << ")";
            }

            if (spawnTimer->isActive()) {
                spawnTimer->stop();
                qDebug() << "Spawn timer stopped";
            }
            QObject::disconnect(spawnTimer, &QTimer::timeout, nullptr, nullptr);
            if (collisionTimer->isActive()) {
                collisionTimer->stop();
                qDebug() << "Collision timer stopped";
            }
            QObject::disconnect(collisionTimer, &QTimer::timeout, nullptr, nullptr);
            qDebug() << "Timers disconnected";

            QList<QGraphicsItem *> items = scenePtr->items();
            for (QGraphicsItem *item : items) {
                if (item == player || item == circleItem || item == backgroundItem) continue;
                if (WarningLaser *warning = dynamic_cast<WarningLaser *>(item)) {
                    warning->stopCountdown();
                    scenePtr->removeItem(warning);
                    delete warning;
                    qDebug() << "WarningLaser deleted during game restart";
                }
                else if (Laser *laser = dynamic_cast<Laser *>(item)) {
                    scenePtr->removeItem(laser);
                    delete laser;
                    qDebug() << "Laser deleted during game restart";
                }
                else {
                    qDebug() << "Unexpected item in scene during restart, removing it";
                    scenePtr->removeItem(item);
                    delete item;
                }
            }

            collidingLaser = nullptr;

            items = scenePtr->items();
            qDebug() << "Items remaining in scene after cleanup:" << items.size();
            for (QGraphicsItem *item : items) {
                qDebug() << "Remaining item type:" << typeid(*item).name();
            }

            setupSpawnTimer();
            setupCollisionTimer();
            QTimer::singleShot(100, [this]() {
                if (!*gameOver) {
                    spawnTimer->setInterval(3000);
                    spawnTimer->start();
                    collisionTimer->start(500);
                    qDebug() << "Timers restarted after delay, first wave interval set to 3000ms";
                }
            });

            qDebug() << "Game restarted";
        }
        else if (event->key() == Qt::Key_Q && gameOver && *gameOver) {
            qDebug() << "Quit requested during Game Over";
            quitRequested = true;
            QApplication::quit();
        }
        QGraphicsView::keyPressEvent(event);
    }

    void closeEvent(QCloseEvent *event) override {
        if (gameOver && *gameOver && !quitRequested) {
            event->ignore();
            qDebug() << "Close event ignored during Game Over";
        } else {
            QGraphicsView::closeEvent(event);
            qDebug() << "Close event accepted";
        }
    }

private:
    bool *gameOver;
    QTimer *spawnTimer;
    QTimer *collisionTimer;
    Player *player;
    int *score;
    int *currentWaveLasers;
    QLabel *scoreLabel;
    QLabel *gameOverLabel;
    QGraphicsScene *scenePtr;
    int circleCenterX;
    int circleCenterY;
    int circleRadius;
    QLabel *countdownLabel;
    QGraphicsEllipseItem *circleItem;
    QGraphicsPixmapItem *backgroundItem;
    bool quitRequested;
    int currentWaveLaserCount;
    int remainingLasersInWave;
    bool isFirstWave;
    Laser *collidingLaser;
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    QWidget startWindow;
    startWindow.setWindowTitle("游戏开始");
    startWindow.setFixedSize(1200, 846);

    QPixmap background(":/new/prefix1/666.jpg");
    if (background.isNull()) {
        qDebug() << "Error: Failed to load 666.jpg for start window background";
    } else {
        QLabel *backgroundLabel = new QLabel(&startWindow);
        backgroundLabel->setPixmap(background);
        backgroundLabel->setGeometry(0, 0, 1200, 846);
        backgroundLabel->lower();
        qDebug() << "Start window background set: original size 1200x846";
    }

    QVBoxLayout *layout = new QVBoxLayout(&startWindow);

    QLabel *welcomeLabel = new QLabel("躲避激光");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("font: 64px; color: white; background-color: rgba(0, 0, 0, 150); padding: 15px; border-radius: 15px;");
    welcomeLabel->setFixedWidth(800);

    QPushButton *startButton = new QPushButton("开始游戏");
    startButton->setStyleSheet(
        "QPushButton { font: 48px; color: white; background-color: rgba(0, 128, 255, 200); padding: 20px; border-radius: 15px; }"
        "QPushButton:hover { background-color: rgba(0, 128, 255, 255); }"
        );
    startButton->setFixedSize(400, 120);

    layout->addStretch();
    layout->addWidget(welcomeLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(70);
    layout->addWidget(startButton, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->setAlignment(Qt::AlignCenter);

    QGraphicsScene scene;
    GameView view(&scene);
    int size = 800;
    view.setFixedSize(size, size);
    scene.setSceneRect(0, 0, size, size);

    // 启用抗锯齿以平滑线条边缘
    view.setRenderHint(QPainter::Antialiasing, true);
    view.setRenderHint(QPainter::SmoothPixmapTransform, true);
    view.setTransform(QTransform());

    qDebug() << "Listing all resources under :/new/prefix1/";
    QDir dir(":/new/prefix1");
    QStringList resourceList = dir.entryList();
    qDebug() << "Resources found:" << resourceList;

    QGraphicsPixmapItem *backgroundItem = nullptr;
    if (!background.isNull()) {
        int scaledWidth = static_cast<int>(background.width() * (800.0 / background.height()));
        int scaledHeight = 800;
        QPixmap scaledBackground = background.scaled(scaledWidth, scaledHeight, Qt::KeepAspectRatio);
        backgroundItem = new QGraphicsPixmapItem(scaledBackground);
        backgroundItem->setPos((800 - scaledWidth) / 2, 0);
        backgroundItem->setZValue(-1);
        scene.addItem(backgroundItem);
        view.setBackgroundItem(backgroundItem);
        qDebug() << "Game background image added to scene: scaled to " << scaledWidth << "x" << scaledHeight;
    }

    int circleRadius = 350;
    int circleCenterX = size / 2;
    int circleCenterY = size / 2;
    QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(circleCenterX - circleRadius, circleCenterY - circleRadius, 2 * circleRadius, 2 * circleRadius);
    circle->setPen(QPen(Qt::black, 2));
    circle->setZValue(0);
    scene.addItem(circle);

    QLabel *countdownLabel = new QLabel(&view);
    countdownLabel->setStyleSheet("font: 24px; color: red;");
    countdownLabel->move(600, 20);
    countdownLabel->adjustSize();
    countdownLabel->show();

    int score = 0;
    QLabel *scoreLabel = new QLabel(&view);
    scoreLabel->setStyleSheet("font: 24px; color: blue;");
    scoreLabel->setText("得分: " + QString::number(score));
    scoreLabel->move(20, 20);
    scoreLabel->adjustSize();
    scoreLabel->show();

    QLabel *gameOverLabel = new QLabel(&view);
    gameOverLabel->setStyleSheet("font: 48px; color: white; text-align: center;");
    gameOverLabel->setText("Game Over\n按 R 键重新开始\n按 Q 键退出");
    gameOverLabel->move(300, 350);
    gameOverLabel->adjustSize();
    gameOverLabel->hide();

    int currentWaveLasers = 0;
    bool gameOver = false;

    QTimer *spawnTimer = new QTimer();
    QTimer *collisionTimer = new QTimer();

    Player *player = new Player();
    player->setCircleBounds(circleCenterX, circleCenterY, circleRadius);
    player->setZValue(1);
    scene.addItem(player);

    view.setGameState(&gameOver, spawnTimer, collisionTimer, player, &score, &currentWaveLasers, scoreLabel, gameOverLabel, &scene, circleCenterX, circleCenterY, circleRadius, countdownLabel, circle);

    view.setupSpawnTimer();
    view.setupCollisionTimer();

    QObject::connect(startButton, &QPushButton::clicked, [&]() {
        startWindow.hide();
        view.show();
        spawnTimer->setInterval(3000);
        spawnTimer->start();
        collisionTimer->start(500);
        qDebug() << "Game started, first wave interval set to 3000ms";
    });

    startWindow.show();
    qDebug() << "Application started with start window";

    return a.exec();
}
