#ifndef GAME_H
#define GAME_H

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QTimer>
#include <QVector>
#include <QWidget>
// Removed QMessageBox as it's no longer directly used by Game class for game
// over display

class Game : public QGraphicsView {
  Q_OBJECT
public:
  Game(QWidget *parent = nullptr);
  virtual ~Game();
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;

  int getScore() const; // Public getter for the score

public slots:
  void update();
  void restartGame();

signals:
  void gameOverSignal(
      int finalScore); // MODIFIED: Signal now carries the final score

private:
  QGraphicsScene *scene;
  QGraphicsPixmapItem *player;
  QVector<QGraphicsRectItem *> platforms;
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
  void resetGame();
};

#endif // GAME_H
