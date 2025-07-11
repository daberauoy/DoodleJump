// --- File: game.cpp ---
#include "game.h"
#include <QDebug>
#include <QFont>
#include <QImage>
#include <QPainter>

Game::Game(Difficulty difficulty, QWidget *parent)
    : QGraphicsView(parent), currentDifficulty(difficulty) {
  scene = new QGraphicsScene(this);
  scene->setSceneRect(0, 0, 400, 600);
  setScene(scene);
  setFixedSize(400, 600);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // MODIFIED: Load player image from "player.png"
  QPixmap playerPixmap("sprites/player.png");
  if (playerPixmap.isNull()) {
    qDebug() << "Error: player.png not found or could not be loaded.";
    // Fallback to a default image if loading fails
    QImage playerImage(45, 45, QImage::Format_ARGB32);
    playerImage.fill(Qt::transparent);
    QPainter painter(&playerImage);
    painter.setBrush(Qt::green);
    painter.drawEllipse(0, 0, 45, 45);
    playerPixmap = QPixmap::fromImage(playerImage);
  }
  player = new QGraphicsPixmapItem(playerPixmap.scaled(
      45, 45, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  scene->addItem(player);
  player->setZValue(1);

  // NEW: Load background image
  QPixmap backgroundPixmap("sprites/background.png");
  if (backgroundPixmap.isNull()) {
    qDebug() << "Error: background.png not found or could not be loaded.";
    // Fallback to a plain background if loading fails
    QImage backgroundImage(400, 600, QImage::Format_ARGB32);
    backgroundImage.fill(Qt::blue);
    backgroundPixmap = QPixmap::fromImage(backgroundImage);
  }
  background = new QGraphicsPixmapItem(backgroundPixmap.scaled(
      400, 600, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
  scene->addItem(background);
  background->setZValue(-1); // Set a low Z-value so it's behind other items

  // MODIFIED: Load platform image once
  platformBasePixmap.load("sprites/platform.png");
  if (platformBasePixmap.isNull()) {
    qDebug() << "Error: platform.png not found or could not be loaded. Using "
                "fallback.";
    // Fallback to a simple white rectangle if image not found
    QImage fallbackPlatformImage(100, 15, QImage::Format_ARGB32);
    fallbackPlatformImage.fill(Qt::white);
    platformBasePixmap = QPixmap::fromImage(fallbackPlatformImage);
  }

  leftPressed = rightPressed = false;
  playerVelocityX = 0;
  playerVelocityY = 0;
  cameraY = 0;

  scoreText = new QGraphicsTextItem();
  scoreText->setDefaultTextColor(Qt::white);
  scoreText->setZValue(100);
  scoreText->setFont(QFont("Arial", 14));
  scene->addItem(scoreText);

  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &Game::update);

  setDifficultyParameters();
  resetGame();
}

Game::~Game() {
  if (timer && timer->isActive()) {
    timer->stop();
  }
  delete timer;
  timer = nullptr;

  // MODIFIED: Platforms are QGraphicsPixmapItem
  for (QGraphicsPixmapItem *platform : platforms) {
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

  // NEW: Delete background item
  if (background) {
    scene->removeItem(background);
    delete background;
    background = nullptr;
  }

  if (scene) {
    delete scene;
    scene = nullptr;
  }
  // platformBasePixmap is a QPixmap object, no explicit deletion needed.
}

int Game::getScore() const { return score; }

void Game::setDifficultyParameters() {
  switch (currentDifficulty) {
  case Easy:
    platformMinYSpacing = 50;
    platformMaxYSpacing = 70;
    platformWidth = 90;
    jumpStrength = -15;
    gravity = 0.4;
    break;
  case Normal:
    platformMinYSpacing = 60;
    platformMaxYSpacing = 80;
    platformWidth = 70;
    jumpStrength = -15;
    gravity = 0.5;
    break;
  case Hard:
    platformMinYSpacing = 70;
    platformMaxYSpacing = 90;
    platformWidth = 50;
    jumpStrength = -16;
    gravity = 0.6;
    break;
  }
}

void Game::resetGame() {
  timer->stop();

  // MODIFIED: Platforms are QGraphicsPixmapItem
  for (QGraphicsPixmapItem *platform : platforms) {
    scene->removeItem(platform);
    delete platform;
  }
  platforms.clear();

  player->setPos(185, 450);
  playerVelocityX = 0;
  playerVelocityY = 0;
  cameraY = 0;
  setSceneRect(0, cameraY, 400, 600);
  // NEW: Reset background position
  background->setPos(0, cameraY);

  score = 0;
  scoreText->setPlainText("Score: 0");
  scoreText->setPos(10, cameraY + 10);

  createPlatforms();

  timer->start(16);
}

void Game::createPlatforms() {
  // MODIFIED: Platforms are QGraphicsPixmapItem
  for (QGraphicsPixmapItem *platform : platforms) {
    scene->removeItem(platform);
    delete platform;
  }
  platforms.clear();

  // MODIFIED: Create first platform using QGraphicsPixmapItem
  QGraphicsPixmapItem *firstPlatform =
      new QGraphicsPixmapItem(platformBasePixmap.scaled(
          platformWidth, 15, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
  firstPlatform->setPos(player->x() -
                            (firstPlatform->boundingRect().width() / 2) +
                            (player->boundingRect().width() / 2),
                        player->y() + player->boundingRect().height() - 2);
  scene->addItem(firstPlatform);
  firstPlatform->setZValue(0);
  platforms.append(firstPlatform);

  int currentY = firstPlatform->y();
  for (int i = 0; i < 4; ++i) {
    // MODIFIED: Create platforms using QGraphicsPixmapItem
    QGraphicsPixmapItem *platform = new QGraphicsPixmapItem(
        platformBasePixmap.scaled(platformWidth, 15, Qt::IgnoreAspectRatio,
                                  Qt::SmoothTransformation));

    currentY -= QRandomGenerator::global()->bounded(platformMaxYSpacing -
                                                    platformMinYSpacing) +
                platformMinYSpacing;

    int x;
    bool overlapping;
    const int maxAttempts = 100;
    int attempts = 0;

    do {
      overlapping = false;
      x = QRandomGenerator::global()->bounded(400 - platformWidth);

      // MODIFIED: Use boundingRect() for QGraphicsPixmapItem for intersection
      // check
      for (QGraphicsPixmapItem *existingPlatform : platforms) {
        if (qAbs(existingPlatform->y() - currentY) < platformMaxYSpacing) {
          QRectF newPlatformRect(x, currentY, platformWidth,
                                 15); // Use platformWidth for new rect
          QRectF existingPlatformRect =
              existingPlatform->mapToScene(existingPlatform->boundingRect())
                  .boundingRect();

          if (newPlatformRect.intersects(existingPlatformRect)) {
            overlapping = true;
            break;
          }
        }
      }
      attempts++;
    } while (overlapping && attempts < maxAttempts);

    platform->setPos(x, currentY);
    scene->addItem(platform);
    platform->setZValue(0);
    platforms.append(platform);
  }
}

void Game::spawnPlatform() {
  // MODIFIED: Platforms are QGraphicsPixmapItem
  for (int i = platforms.size() - 1; i >= 0; --i) {
    if (platforms[i]->y() > (cameraY + scene->height() + 50)) {
      scene->removeItem(platforms[i]);
      delete platforms[i];
      platforms.remove(i);
    }
  }

  while (platforms.size() < 15) {
    // MODIFIED: Create platforms using QGraphicsPixmapItem
    QGraphicsPixmapItem *platform = new QGraphicsPixmapItem(
        platformBasePixmap.scaled(platformWidth, 15, Qt::IgnoreAspectRatio,
                                  Qt::SmoothTransformation));

    int highestPlatformY =
        platforms.isEmpty() ? cameraY : platforms.first()->y();
    int y = highestPlatformY - (QRandomGenerator::global()->bounded(
                                    platformMaxYSpacing - platformMinYSpacing) +
                                platformMinYSpacing);

    int x;
    bool overlapping;
    const int maxAttempts = 100;
    int attempts = 0;

    do {
      overlapping = false;
      x = QRandomGenerator::global()->bounded(400 - platformWidth);

      // MODIFIED: Use boundingRect() for QGraphicsPixmapItem for intersection
      // check
      for (QGraphicsPixmapItem *existingPlatform : platforms) {
        if (qAbs(existingPlatform->y() - y) < platformMaxYSpacing) {
          QRectF newPlatformRect(x, y, platformWidth,
                                 15); // Use platformWidth for new rect
          QRectF existingPlatformRect =
              existingPlatform->mapToScene(existingPlatform->boundingRect())
                  .boundingRect();

          if (newPlatformRect.intersects(existingPlatformRect)) {
            overlapping = true;
            break;
          }
        }
      }
      attempts++;
    } while (overlapping && attempts < maxAttempts);

    platform->setPos(x, y);

    scene->addItem(platform);
    platform->setZValue(0);
    platforms.prepend(platform);
  }
}

void Game::checkCollisions() {
  bool onPlatform = false;
  QRectF playerRect = player->mapToScene(player->boundingRect()).boundingRect();

  // MODIFIED: Iterate over QGraphicsPixmapItem for platforms
  for (QGraphicsPixmapItem *platform : platforms) {
    QRectF platformRect =
        platform->mapToScene(platform->boundingRect()).boundingRect();

    if (playerVelocityY > 0 && playerRect.bottom() <= platformRect.top() + 5 &&
        playerRect.right() > platformRect.left() + 5 &&
        playerRect.left() < platformRect.right() - 5 &&
        playerRect.bottom() + playerVelocityY > platformRect.top()) {
      playerVelocityY = jumpStrength;
      onPlatform = true;
    }
  }

  if (!onPlatform) {
    playerVelocityY += gravity;
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
    // NEW: Adjust background position with camera
    background->setY(cameraY);
  }

  setSceneRect(0, cameraY, 400, 600);

  if (-cameraY > score) {
    score = -cameraY;
    scoreText->setPlainText("Score: " + QString::number(score));
  }
  scoreText->setPos(10, cameraY + 10);

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
  emit gameOverSignal(score);
}

void Game::restartGame() { resetGame(); }
