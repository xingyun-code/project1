#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "player.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QGraphicsScene scene;
    QGraphicsView view(&scene);

    view.setFixedSize(800, 600);
    scene.setSceneRect(0, 0, 800, 600);

    Player *player = new Player();
    scene.addItem(player);

    view.show();
    return a.exec();
}
