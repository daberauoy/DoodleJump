#include "game.h"
#include <QDebug>
#include <QFont>
#include <QImage>
#include <QMessageBox>
#include <QPainter>

Game::Game(QWidget *parent) : QGraphicsView(parent) {
  scene = new QGraphicsScene(this);
  scene->setSceneRect(0, 0, 400, 600);
  setScene(scene);
  setFixedSize(400, 600);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  QImage playerImage(30, 30, QImage::Format_ARGB32);
  playerImage.fill(Qt::transparent);
  QPainter painter(&playerImage);
  painter.setBrush(Qt::green);
  painter.drawEllipse(0, 0, 30, 30);
  player = new QGraphicsPixmapItem(QPixmap::fromImage(playerImage));
  scene->addItem(player);

  leftPressed = rightPressed = false;
  playerVelocityX = 0;
  playerVelocityY = 0;
  cameraY = 0;

  scoreText = new QGraphicsTextItem();
  scoreText->setDefaultTextColor(Qt::white);
  scoreText->setZValue(100);
  scene->addItem(scoreText);

  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &Game::update);

  resetGame();
}

Game::~Game() {
  if (timer && timer->isActive()) {
    timer->stop();
  }
  delete timer;
  timer = nullptr;

  for (QGraphicsRectItem *platform : platforms) {
    scene->removeItem(platform);
    delete platform;
  }
  platforms.clear();

  if (player) {
    scene->removeItem(player);
    delete player;
    player = nullptr;
  }

  if (scoreText) {
    scene->removeItem(scoreText);
    delete scoreText;
    scoreText = nullptr;
  }

  if (scene) {
    delete scene;
    scene = nullptr;
  }
}

// NEW: Implementation of the getScore getter
int Game::getScore() const { return score; }

void Game::resetGame() {
  timer->stop();

  for (QGraphicsRectItem *platform : platforms) {
    scene->removeItem(platform);
    delete platform;
  }
  platforms.clear();

  player->setPos(185, 500);
  playerVelocityX = 0;
  playerVelocityY = 0;
  cameraY = 0;
  setSceneRect(0, cameraY, 400, 600);

  score = 0;
  scoreText->setPlainText("Score: 0");
  scoreText->setPos(10, 10);

  createPlatforms();

  timer->start(16);
}

void Game::createPlatforms() {
  QGraphicsRectItem *initialPlatform = new QGraphicsRectItem(0, 0, 70, 15);
  initialPlatform->setBrush(Qt::green);
  initialPlatform->setPos(player->x() - (initialPlatform->rect().width() / 2) +
                              (player->boundingRect().width() / 2),
                          player->y() + player->boundingRect().height());
  scene->addItem(initialPlatform);
  platforms.append(initialPlatform);

  for (int i = 0; i < 9; ++i) {
    QGraphicsRectItem *platform = new QGraphicsRectItem(0, 0, 70, 15);
    platform->setBrush(Qt::white);

    int x = QRandomGenerator::global()->bounded(330);
    int y = initialPlatform->y() - (i + 1) * 60 -
            QRandomGenerator::global()->bounded(20);
    platform->setPos(x, y);

    scene->addItem(platform);
    platforms.append(platform);
  }
}

void Game::spawnPlatform() {
  for (int i = platforms.size() - 1; i >= 0; --i) {
    if (platforms[i]->y() > (cameraY + scene->height())) {
      scene->removeItem(platforms[i]);
      delete platforms[i];
      platforms.remove(i);
    }
  }

  while (platforms.size() < 10) {
    QGraphicsRectItem *platform = new QGraphicsRectItem(0, 0, 70, 15);
    platform->setBrush(Qt::white);

    int x = QRandomGenerator::global()->bounded(330);
    int highestPlatformY =
        platforms.isEmpty() ? cameraY : platforms.last()->y();
    int y = highestPlatformY - QRandomGenerator::global()->bounded(60) - 50;

    platform->setPos(x, y);

    scene->addItem(platform);
    platforms.append(platform);
  }
}

void Game::checkCollisions() {
  bool onPlatform = false;
  QRectF playerRect = player->mapToScene(player->boundingRect()).boundingRect();

  for (QGraphicsRectItem *platform : platforms) {
    QRectF platformRect = platform->mapToScene(platform->rect()).boundingRect();

    if (playerVelocityY > 0 && playerRect.bottom() <= platformRect.top() + 5 &&
        playerRect.right() > platformRect.left() + 5 &&
        playerRect.left() < platformRect.right() - 5 &&
        playerRect.bottom() + playerVelocityY > platformRect.top()) {
      playerVelocityY = -15; // Jump
      onPlatform = true;
    }
  }

  if (!onPlatform) {
    playerVelocityY += 0.5;
  }
}

void Game::update() {
  if (!timer || !timer->isActive()) {
    return;
  }

  if (leftPressed)
    playerVelocityX = -5;
  else if (rightPressed)
    playerVelocityX = 5;
  else
    playerVelocityX *= 0.9;

  player->setX(player->x() + playerVelocityX);

  if (player->x() < 0)
    player->setX(scene->width() - player->boundingRect().width());
  if (player->x() > scene->width() - player->boundingRect().width())
    player->setX(0);

  checkCollisions();

  player->setY(player->y() + playerVelocityY);

  const int targetPlayerYInView = 200;
  int playerYInScene = player->y();

  if (playerYInScene < cameraY + targetPlayerYInView) {
    int delta = (cameraY + targetPlayerYInView) - playerYInScene;
    cameraY -= delta;
  }

  setSceneRect(0, cameraY, 400, 600);

  if (-cameraY > score) {
    score = -cameraY;
    scoreText->setPlainText("Score: " + QString::number(score));
  }
  scoreText->setPos(cameraY + 10, cameraY + 10);

  spawnPlatform();

  if (player->y() > (cameraY + scene->height())) {
    gameOver();
  }
}

void Game::keyPressEvent(QKeyEvent *event) {
  if (timer && timer->isActive()) {
    if (event->key() == Qt::Key_Left) {
      leftPressed = true;
    } else if (event->key() == Qt::Key_Right) {
      rightPressed = true;
    }
  }
  QGraphicsView::keyPressEvent(event);
}

void Game::keyReleaseEvent(QKeyEvent *event) {
  if (timer && timer->isActive()) {
    if (event->key() == Qt::Key_Left) {
      leftPressed = false;
    } else if (event->key() == Qt::Key_Right) {
      rightPressed = false;
    }
  }
  QGraphicsView::keyReleaseEvent(event);
}

void Game::gameOver() {
  timer->stop();
  emit gameOverSignal();
}

void Game::restartGame() { resetGame(); }
