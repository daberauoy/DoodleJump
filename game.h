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

// Enum to define difficulty levels
enum Difficulty { Easy, Normal, Hard };

class Game : public QGraphicsView {
  Q_OBJECT
public:
  // MODIFIED: Constructor now accepts a Difficulty parameter
  Game(Difficulty difficulty, QWidget *parent = nullptr);
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

  // NEW: Background item
  QGraphicsPixmapItem *background;

  // NEW: Difficulty member variable
  Difficulty currentDifficulty;
  // NEW: Difficulty-dependent parameters
  int platformMinYSpacing;
  int platformMaxYSpacing;
  int platformWidth;
  double jumpStrength;
  double gravity;

  void createPlatforms();
  void spawnPlatform();
  void checkCollisions();
  void gameOver();
  void resetGame();
  // NEW: Helper function to set game parameters based on difficulty
  void setDifficultyParameters();
};

#endif // GAME_H
