#ifndef GAME_H
#define GAME_H

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMessageBox> // Include QMessageBox
#include <QPainter>
#include <QRandomGenerator>
#include <QTimer>
#include <QVector>
#include <QWidget>

class Game : public QGraphicsView {
  Q_OBJECT
public:
  Game(QWidget *parent = nullptr);
  virtual ~Game();
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;

  // NEW: Public getter for the score
  int getScore() const; // Add this line

public slots:
  void update();
  void restartGame();

signals:
  void gameOverSignal(); // Signal to notify MainWindow that game is over

private:
  QGraphicsScene *scene;
  QGraphicsPixmapItem *player;
  QVector<QGraphicsRectItem *> platforms;
  QTimer *timer;
  bool leftPressed, rightPressed;
  double playerVelocityX, playerVelocityY;
  int cameraY;
  int score; // This remains private
  QGraphicsTextItem *scoreText;

  void createPlatforms();
  void spawnPlatform();
  void checkCollisions();
  void gameOver();
  void resetGame();
};

#endif // GAME_H
