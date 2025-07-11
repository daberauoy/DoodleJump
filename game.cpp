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
  scoreText->setFont(QFont("Arial", 12)); // Set a font for better visibility
  scene->addItem(scoreText);

  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &Game::update);

  setDifficultyParameters(); // NEW: Set parameters based on chosen difficulty
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

int Game::getScore() const { return score; }

// NEW: Function to set game parameters based on difficulty
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
    jumpStrength =
        -16; // Slightly weaker jump or higher initial velocity needed
    gravity = 0.6;
    break;
  }
}

void Game::resetGame() {
  timer->stop();

  for (QGraphicsRectItem *platform : platforms) {
    scene->removeItem(platform);
    delete platform;
  }
  platforms.clear();

  player->setPos(185, 450); // Set player to a fixed start position, Y=450 to
                            // align with initial platforms
  playerVelocityX = 0;
  playerVelocityY = 0;
  cameraY = 0;
  setSceneRect(0, cameraY, 400, 600);

  score = 0;
  scoreText->setPlainText("Score: 0");
  scoreText->setPos(10, cameraY + 10);

  createPlatforms();

  timer->start(16);
}

void Game::createPlatforms() {
  // Clear existing platforms before creating new ones (important for restart)
  for (QGraphicsRectItem *platform : platforms) {
    scene->removeItem(platform);
    delete platform;
  }
  platforms.clear();

  // Create the first platform directly below the player's initial position
  // Adjusted Y to be precisely where the player would land initially
  QGraphicsRectItem *firstPlatform =
      new QGraphicsRectItem(0, 0, platformWidth, 15);
  firstPlatform->setBrush(Qt::green);
  firstPlatform->setPos(player->x() - (firstPlatform->rect().width() / 2) +
                            (player->boundingRect().width() / 2),
                        player->y() + player->boundingRect().height() -
                            2); // Minor adjustment for landing
  scene->addItem(firstPlatform);
  platforms.append(firstPlatform);

  // Create subsequent platforms above the first one, ensuring a consistent
  // density
  int currentY = firstPlatform->y();
  // Only create 4 additional platforms for a total of 5 at the start
  for (int i = 0; i < 4; ++i) {
    QGraphicsRectItem *platform =
        new QGraphicsRectItem(0, 0, platformWidth, 15);
    platform->setBrush(Qt::white);

    // Calculate Y for the new platform
    currentY -= QRandomGenerator::global()->bounded(platformMaxYSpacing -
                                                    platformMinYSpacing) +
                platformMinYSpacing;

    // Try to find a non-overlapping X position
    int x;
    bool overlapping;
    const int maxAttempts = 100; // Prevent infinite loop in rare cases
    int attempts = 0;

    do {
      overlapping = false;
      x = QRandomGenerator::global()->bounded(
          400 - platformWidth); // Generate a new X

      // Check against existing platforms for horizontal overlap at similar Y
      // levels Only check platforms roughly in the same vertical vicinity
      for (QGraphicsRectItem *existingPlatform : platforms) {
        // Check if the existing platform is within a vertical range
        if (qAbs(existingPlatform->y() - currentY) < platformMaxYSpacing) {
          QRectF newPlatformRect(x, currentY, platformWidth, 15);
          QRectF existingPlatformRect =
              existingPlatform->mapToScene(existingPlatform->rect())
                  .boundingRect();

          if (newPlatformRect.intersects(existingPlatformRect)) {
            overlapping = true;
            break; // Overlap found, try a new X
          }
        }
      }
      attempts++;
    } while (overlapping && attempts < maxAttempts);

    platform->setPos(x, currentY);
    scene->addItem(platform);
    platforms.append(platform);
  }
}

void Game::spawnPlatform() {
  // Remove platforms that have gone too far down (off-screen below the camera)
  for (int i = platforms.size() - 1; i >= 0; --i) {
    if (platforms[i]->y() >
        (cameraY + scene->height() + 50)) { // Added 50 pixel buffer
      scene->removeItem(platforms[i]);
      delete platforms[i];
      platforms.remove(i);
    }
  }

  // Add new platforms until we have a desired number (e.g., 10-15 platforms on
  // screen)
  while (platforms.size() <
         15) { // Increased minimum platforms to keep the screen fuller
    QGraphicsRectItem *platform =
        new QGraphicsRectItem(0, 0, platformWidth, 15);
    platform->setBrush(Qt::white);

    // Find the highest platform's Y-coordinate to place new platforms above it
    int highestPlatformY =
        platforms.isEmpty() ? cameraY : platforms.first()->y();
    // Place new platform above the highest existing one, within defined spacing
    int y = highestPlatformY - (QRandomGenerator::global()->bounded(
                                    platformMaxYSpacing - platformMinYSpacing) +
                                platformMinYSpacing);

    // Try to find a non-overlapping X position
    int x;
    bool overlapping;
    const int maxAttempts = 100; // Prevent infinite loop in rare cases
    int attempts = 0;

    do {
      overlapping = false;
      x = QRandomGenerator::global()->bounded(
          400 - platformWidth); // Generate a new X

      // Check against existing platforms for horizontal overlap at similar Y
      // levels Only check platforms roughly in the same vertical vicinity
      for (QGraphicsRectItem *existingPlatform : platforms) {
        // Check if the existing platform is within a vertical range where
        // overlap is possible
        if (qAbs(existingPlatform->y() - y) <
            platformMaxYSpacing) { // Use platformMaxYSpacing as a general range
          QRectF newPlatformRect(x, y, platformWidth, 15);
          QRectF existingPlatformRect =
              existingPlatform->mapToScene(existingPlatform->rect())
                  .boundingRect();

          if (newPlatformRect.intersects(existingPlatformRect)) {
            overlapping = true;
            break; // Overlap found, try a new X
          }
        }
      }
      attempts++;
    } while (overlapping && attempts < maxAttempts);

    platform->setPos(x, y);

    scene->addItem(platform);
    // Insert new platforms at the beginning of the list to keep it sorted by Y
    // (highest first)
    platforms.prepend(platform);
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
      playerVelocityY =
          jumpStrength; // Jump strength is now difficulty-dependent
      onPlatform = true;
    }
  }

  if (!onPlatform) {
    playerVelocityY += gravity; // Gravity is now difficulty-dependent
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

  // Score should increase as the camera moves up (player goes higher)
  // The score is the negative of the camera's Y position.
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
