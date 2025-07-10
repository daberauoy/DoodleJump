#ifndef GAME_H
#define GAME_H

#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget> // Add this include for QGraphicsProxyWidget
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPainter>
#include <QPushButton>
#include <QRandomGenerator>
#include <QTimer>
#include <QVector>
#include <QWidget>

class Game : public QGraphicsView {
  Q_OBJECT
public:
  Game(QWidget *parent = nullptr);
  virtual ~Game(); // DECLARATION: The virtual destructor is declared here
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;

public slots:
  void update();
  void restartGame(); // New slot for restarting the game

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
  QGraphicsTextItem *gameOverText;          // Store Game Over text to hide/show
  QPushButton *restartButton;               // Store restart button to hide/show
  QGraphicsProxyWidget *restartButtonProxy; // Store the proxy widget

  void createPlatforms();
  void spawnPlatform();
  void checkCollisions();
  void gameOver();
  void resetGame(); // New private helper to reset game state
};

#endif // GAME_H
