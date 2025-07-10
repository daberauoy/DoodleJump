#ifndef GAME_H
#define GAME_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QTimer>
#include <QVector>
#include <QRandomGenerator>

class Game : public QGraphicsView
{
    Q_OBJECT
public:
    Game(QWidget *parent = nullptr);
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

public slots:
    void update();

private:
    QGraphicsScene *scene;
    QGraphicsPixmapItem *player;
    QVector<QGraphicsRectItem*> platforms;
    QTimer *timer;
    bool leftPressed, rightPressed;
    double playerVelocityX, playerVelocityY;
    int cameraY;
    int score;
    QGraphicsTextItem *scoreText;

    void createPlatforms();
    void spawnPlatform();
    void checkCollisions();
    void gameOver();
};

#endif // GAME_H
