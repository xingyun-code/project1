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
#include "player.h"
#include "warninglaser.h"
#include "laser.h"

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
        quitRequested = false;
        qDebug() << "GameView constructed";
    }

    ~GameView() {
        for (QTimer *timer : laserTimers) {
            timer->stop();
            timer->deleteLater();
        }
        laserTimers.clear();
        qDebug() << "GameView destroyed, laser timers cleaned up";
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
            qreal angleStep = 360.0 / laserCount;

            for (int i = 0; i < laserCount; ++i) {
                qreal startAngle = i * angleStep;
                qreal startRad = qDegreesToRadians(startAngle);
                qreal startX = circleCenterX + circleRadius * std::cos(startRad);
                qreal startY = circleCenterY + circleRadius * std::sin(startRad);

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

                qreal endX = circleCenterX + circleRadius * dx;
                qreal endY = circleCenterY + circleRadius * dy;
                qreal warningLength = (2 * circleRadius) / 8;
                qreal warningEndX = startX + warningLength * dx;
                qreal warningEndY = startY + warningLength * dy;

                startX = std::round(startX);
                startY = std::round(startY);
                warningEndX = std::round(warningEndX);
                warningEndY = std::round(warningEndY);
                endX = std::round(endX);
                endY = std::round(endY);

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

                QObject::connect(warning, &WarningLaser::countdownFinished, [startX, startY, endX, endY, dx, dy, this, warning]() {
                    if (*gameOver || !scenePtr) {
                        qDebug() << "Countdown finished but game is over or scenePtr is invalid";
                        return;
                    }

                    Laser *laser = new Laser();
                    laser->setLine(startX, startY, endX, endY);
                    scenePtr->addItem(laser);
                    qDebug() << "Laser created at (" << startX << "," << startY << ") to (" << endX << "," << endY << ") with direction (" << dx << "," << dy << ")";

                    QTimer *laserTimer = new QTimer();
                    laserTimer->setSingleShot(true);
                    laserTimers.append(laserTimer);
                    QObject::connect(laserTimer, &QTimer::timeout, [laser, this, laserTimer]() {
                        if (laser->scene()) {
                            laser->scene()->removeItem(laser);
                        }
                        delete laser;
                        qDebug() << "Laser deleted";

                        laserTimers.removeOne(laserTimer);
                        laserTimer->deleteLater();

                        (*currentWaveLasers)--;
                        if (*currentWaveLasers == 0 && !*gameOver) {
                            (*score)++;
                            scoreLabel->setText("得分: " + QString::number(*score));
                            scoreLabel->adjustSize();
                            qDebug() << "Score updated:" << *score;
                        }
                    });
                    laserTimer->start(2000);

                    if (warning->scene()) {
                        scenePtr->removeItem(warning);
                    }
                    delete warning;
                    qDebug() << "WarningLaser deleted";
                });
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

                    for (QTimer *timer : laserTimers) {
                        timer->stop();
                        timer->deleteLater();
                    }
                    laserTimers.clear();
                    qDebug() << "Laser timers stopped and cleared during Game Over";

                    for (QGraphicsItem *itemToRemove : scenePtr->items()) {
                        if (itemToRemove == player || itemToRemove == circleItem) continue;
                        if (WarningLaser *warning = dynamic_cast<WarningLaser *>(itemToRemove)) {
                            warning->stopCountdown();
                            scenePtr->removeItem(warning);
                            delete warning;
                            qDebug() << "WarningLaser deleted during Game Over cleanup";
                        }
                        else if (Laser *laser = dynamic_cast<Laser *>(itemToRemove)) {
                            scenePtr->removeItem(laser);
                            delete laser;
                            qDebug() << "Laser deleted during Game Over cleanup";
                        }
                    }
                    qDebug() << "Game Over: Player collided with laser";
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

            for (QTimer *timer : laserTimers) {
                timer->stop();
                timer->deleteLater();
            }
            laserTimers.clear();
            qDebug() << "Laser timers stopped and cleared during restart";

            QList<QGraphicsItem *> items = scenePtr->items();
            for (QGraphicsItem *item : items) {
                if (item == player || item == circleItem) continue;
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

            items = scenePtr->items();
            qDebug() << "Items remaining in scene after cleanup:" << items.size();
            for (QGraphicsItem *item : items) {
                qDebug() << "Remaining item type:" << typeid(*item).name();
            }

            setupSpawnTimer();
            setupCollisionTimer();
            QTimer::singleShot(100, [this]() {
                if (!*gameOver) {
                    spawnTimer->start(5000);
                    collisionTimer->start(500);
                    qDebug() << "Timers restarted after delay";
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
    QList<QTimer*> laserTimers;
    bool quitRequested;
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    QGraphicsScene scene;
    GameView view(&scene);
    int size = 800;
    view.setFixedSize(size, size);
    scene.setSceneRect(0, 0, size, size);

    int circleRadius = 350;
    int circleCenterX = size / 2;
    int circleCenterY = size / 2;
    QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(circleCenterX - circleRadius, circleCenterY - circleRadius, 2 * circleRadius, 2 * circleRadius);
    circle->setPen(QPen(Qt::black, 2));
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
    gameOverLabel->setStyleSheet("font: 48px; color: red; text-align: center;");
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
    scene.addItem(player);

    view.setGameState(&gameOver, spawnTimer, collisionTimer, player, &score, &currentWaveLasers, scoreLabel, gameOverLabel, &scene, circleCenterX, circleCenterY, circleRadius, countdownLabel, circle);

    view.setupSpawnTimer();
    view.setupCollisionTimer();
    spawnTimer->start(5000);
    collisionTimer->start(500);

    view.show();
    qDebug() << "Application started";
    return a.exec();
}
