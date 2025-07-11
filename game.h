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

enum Difficulty { Easy, Normal, Hard };

class Game : public QGraphicsView {
  Q_OBJECT
public:
  Game(Difficulty difficulty, QWidget *parent = nullptr);
  virtual ~Game();
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;

  int getScore() const;

public slots:
  void update();
  void restartGame();

signals:
  void gameOverSignal(int finalScore);

private:
  QGraphicsScene *scene;
  QGraphicsPixmapItem *player;
  QVector<QGraphicsPixmapItem *> platforms;
  QTimer *timer;
  bool leftPressed, rightPressed;
  double playerVelocityX, playerVelocityY;
  int cameraY;
  int score;
  QGraphicsTextItem *scoreText;

  QGraphicsPixmapItem *background;

  Difficulty currentDifficulty;
  int platformMinYSpacing;
  int platformMaxYSpacing;
  int platformWidth;
  double jumpStrength;
  double gravity;

  QPixmap platformBasePixmap;

  void createPlatforms();
  void spawnPlatform();
  void checkCollisions();
  void gameOver();
  void resetGame();
  void setDifficultyParameters();
};

#endif
