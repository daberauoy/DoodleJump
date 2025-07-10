Winclude "game.h"
#include <QDebug>
#include <QFont>
#include <QGraphicsProxyWidget>
#include <QImage>
#include <QPainter>

    Game::Game(QWidget *parent)
    : QGraphicsView(parent) {
  // Scene setup
  scene = new QGraphicsScene(this);
  scene->setSceneRect(0, 0, 400, 600); // Fixed scene size for the game area
  setScene(scene);
  setFixedSize(400, 600); // Fixed view size
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // Player creation
  QImage playerImage(30, 30, QImage::Format_ARGB32);
  playerImage.fill(Qt::transparent);
  QPainter painter(&playerImage);
  painter.setBrush(Qt::green);
  painter.drawEllipse(0, 0, 30, 30);
  player = new QGraphicsPixmapItem(QPixmap::fromImage(playerImage));
  scene->addItem(player);

  // Initialize game variables
  leftPressed = rightPressed = false;
  playerVelocityX = 0;
  playerVelocityY = 0;
  cameraY = 0; // Represents the top of the visible screen in scene coordinates

  // Score text setup
  scoreText = new QGraphicsTextItem();
  scoreText->setDefaultTextColor(Qt::white);
  scoreText->setZValue(100); // Ensure score is on top of other items
  scene->addItem(scoreText);

  // Initialize game over elements to null
  gameOverText = nullptr;
  restartButton = nullptr;
  restartButtonProxy = nullptr;

  // Game timer setup
  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &Game::update);

  // Initial game setup
  resetGame();
}

// Destructor for proper cleanup
Game::~Game() {
  // Stop and delete the timer
  if (timer && timer->isActive()) {
    timer->stop();
  }
  delete timer;
  timer = nullptr;

  // Delete all platforms
  for (QGraphicsRectItem *platform : platforms) {
    scene->removeItem(platform);
    delete platform;
  }
  platforms.clear();

  // Delete player
  if (player) {
    scene->removeItem(player);
    delete player;
    player = nullptr;
  }

  // Delete score text
  if (scoreText) {
    scene->removeItem(scoreText);
    delete scoreText;
    scoreText = nullptr;
  }

  // Delete game over text
  if (gameOverText) {
    scene->removeItem(gameOverText);
    delete gameOverText;
    gameOverText = nullptr;
  }

  // Delete restart button proxy and the button it contains
  if (restartButtonProxy) {
    scene->removeItem(restartButtonProxy);
    delete restartButtonProxy; // This correctly deletes the
                               // QGraphicsProxyWidget AND the QWidget it holds
                               // (restartButton)
    restartButtonProxy = nullptr;
    restartButton = nullptr; // Set button pointer to null as it's now deleted
  } else if (restartButton) {
    // Fallback: If for some reason proxy was not created but button exists,
    // delete it
    delete restartButton;
    restartButton = nullptr;
  }

  // Delete the scene
  if (scene) {
    delete scene;
    scene = nullptr;
  }
}

void Game::resetGame() {
  // Stop timer during reset to prevent update calls on partially reset state
  timer->stop();

  // Clear existing platforms
  for (QGraphicsRectItem *platform : platforms) {
    scene->removeItem(platform);
    delete platform;
  }
  platforms.clear();

  // Reset player position and velocities
  player->setPos(185, 500); // Start near the bottom of the initial screen
  playerVelocityX = 0;
  playerVelocityY = 0;
  cameraY = 0;                        // Reset camera to the initial view
  setSceneRect(0, cameraY, 400, 600); // Update the scene's viewable rectangle

  // Reset score
  score = 0;
  scoreText->setPlainText("Score: 0");
  // Position score text relative to the current camera view
  // scoreText->setPos(cameraY + 10, cameraY + 10); // This was already correct
  // for score
  scoreText->setPos(10,
                    10); // Position score relative to the top-left of the view

  // Remove and delete Game Over text if it exists
  if (gameOverText) {
    scene->removeItem(gameOverText);
    delete gameOverText;
    gameOverText = nullptr;
  }

  // Remove and delete restart button and its proxy if they exist
  if (restartButtonProxy) {
    scene->removeItem(restartButtonProxy);
    delete restartButtonProxy;
    restartButtonProxy = nullptr;
    restartButton = nullptr; // The button is deleted when the proxy is deleted
  }

  // Create initial platforms for the new game
  createPlatforms();

  // Start the game timer for a new round
  timer->start(16); // ~60 FPS
}

void Game::createPlatforms() {
  // Create the first platform directly below the player's initial position
  QGraphicsRectItem *initialPlatform = new QGraphicsRectItem(0, 0, 70, 15);
  initialPlatform->setBrush(Qt::green);
  initialPlatform->setPos(player->x() - (initialPlatform->rect().width() / 2) +
                              (player->boundingRect().width() / 2),
                          player->y() + player->boundingRect().height());
  scene->addItem(initialPlatform);
  platforms.append(initialPlatform);

  // Create additional platforms above the initial one
  for (int i = 0; i < 9; ++i) { // 9 more platforms, total 10
    QGraphicsRectItem *platform = new QGraphicsRectItem(0, 0, 70, 15);
    platform->setBrush(Qt::white);

    int x = QRandomGenerator::global()->bounded(330); // Random X position
    // Place platforms above the previous one with some vertical spacing
    int y = initialPlatform->y() - (i + 1) * 60 -
            QRandomGenerator::global()->bounded(20);
    platform->setPos(x, y);

    scene->addItem(platform);
    platforms.append(platform);
  }
}

void Game::spawnPlatform() {
  // Remove platforms that have fallen off the bottom of the screen
  for (int i = platforms.size() - 1; i >= 0; --i) {
    if (platforms[i]->y() >
        (cameraY + scene->height())) { // If platform is below current view
      scene->removeItem(platforms[i]);
      delete platforms[i];
      platforms.remove(i);
    }
  }

  // Add new platforms above the current camera view if there are less than 10
  while (platforms.size() < 10) {
    QGraphicsRectItem *platform = new QGraphicsRectItem(0, 0, 70, 15);
    platform->setBrush(Qt::white);

    int x = QRandomGenerator::global()->bounded(330);
    // Get the Y-coordinate of the highest (lowest Y value) platform
    int highestPlatformY =
        platforms.isEmpty() ? cameraY : platforms.last()->y();
    // Place new platforms randomly above the highest existing one
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

    // Check for collision when player is falling and lands on a platform
    if (playerVelocityY > 0 && // Player is falling
        playerRect.bottom() <=
            platformRect.top() +
                5 && // Player is just above or touching platform top
        playerRect.right() >
            platformRect.left() +
                5 && // Player is horizontally within platform bounds
        playerRect.left() < platformRect.right() - 5 &&
        playerRect.bottom() + playerVelocityY >
            platformRect.top()) // Player will land on platform in next step
    {
      playerVelocityY = -15; // Make player jump
      onPlatform = true;
    }
  }

  // Apply gravity if not on a platform
  if (!onPlatform) {
    playerVelocityY += 0.5;
  }
}

void Game::update() {
  // Don't update game state if the timer is not active (game is paused/over)
  if (!timer || !timer->isActive()) {
    return;
  }

  // Player horizontal movement
  if (leftPressed)
    playerVelocityX = -5;
  else if (rightPressed)
    playerVelocityX = 5;
  else
    playerVelocityX *= 0.9; // Apply friction

  player->setX(player->x() + playerVelocityX);

  // Wrap player around screen horizontally
  if (player->x() < 0)
    player->setX(scene->width() - player->boundingRect().width());
  if (player->x() > scene->width() - player->boundingRect().width())
    player->setX(0);

  // Check for collisions and apply gravity
  checkCollisions();

  // Update player vertical position
  player->setY(player->y() + playerVelocityY);

  // Camera movement logic: Keep player in upper part of the screen when
  // ascending
  const int targetPlayerYInView = 200; // Player should ideally be around 200
                                       // pixels from the top of the view
  int playerYInScene =
      player->y(); // Player's current Y-coordinate in scene space

  // If player goes above the target Y in the view, shift camera up
  if (playerYInScene < cameraY + targetPlayerYInView) {
    int delta = (cameraY + targetPlayerYInView) - playerYInScene;
    cameraY -= delta; // Move camera origin (top of the scene visible) up
  }

  // Apply the new camera position to the scene view
  setSceneRect(0, cameraY, 400, 600);

  // Update score based on how high the camera has moved
  if (-cameraY > score) {
    score = -cameraY;
    scoreText->setPlainText("Score: " + QString::number(score));
  }
  // Reposition score text to always appear at the top-left of the *current
  // visible view* This is relative to the viewport, not shifted by cameraY
  scoreText->setPos(cameraY + 10, cameraY + 10); // Keep score fixed in the view

  spawnPlatform();

  // Check for game over (player falls below the visible screen)
  if (player->y() > (cameraY + scene->height())) {
    gameOver();
  }
}

void Game::keyPressEvent(QKeyEvent *event) {
  // Only process player movement key events if the game is active
  if (timer && timer->isActive()) {
    if (event->key() == Qt::Key_Left) {
      leftPressed = true;
    } else if (event->key() == Qt::Key_Right) {
      rightPressed = true;
    }
  }
  // Call base class keyPressEvent to allow other widgets (like buttons) to
  // receive events
  QGraphicsView::keyPressEvent(event);
}

void Game::keyReleaseEvent(QKeyEvent *event) {
  // Only process player movement key events if the game is active
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
  timer->stop(); // Stop the game loop when game is over

  // Calculate center coordinates for UI elements relative to the *fixed
  // viewport* These coordinates are independent of cameraY.
  qreal viewWidth = 400;  // Fixed view width
  qreal viewHeight = 600; // Fixed view height
  qreal viewportCenterX = viewWidth / 2.0;
  qreal viewportCenterY = viewHeight / 2.0;

  // Create and position Game Over text
  if (!gameOverText) {
    gameOverText = new QGraphicsTextItem();
    gameOverText->setDefaultTextColor(Qt::red);
    gameOverText->setFont(QFont("Arial", 24));
    scene->addItem(gameOverText);
    gameOverText->setZValue(100); // Ensure it's on top
  }
  gameOverText->setPlainText("Game Over\nScore: " + QString::number(score));
  // Position text in the center of the viewport, adjusted for its size
  gameOverText->setPos(cameraY + viewportCenterX -
                           (gameOverText->boundingRect().width() / 2.0),
                       cameraY + viewportCenterY - 50); // Slightly above center
  gameOverText->setVisible(true);

  // Create and position Restart Game button
  if (!restartButton) { // Only create the button and proxy once
    restartButton = new QPushButton("Restart Game");
    restartButton->setFont(QFont("Arial", 16));
    restartButton->setFixedSize(150, 50); // Set button's size
    connect(restartButton, &QPushButton::clicked, this, &Game::restartGame);

    restartButtonProxy = new QGraphicsProxyWidget(); // Create the proxy
    restartButtonProxy->setWidget(
        restartButton);                 // Set the button as its widget
    scene->addItem(restartButtonProxy); // Add the proxy to the scene
    restartButtonProxy->setZValue(100); // Ensure it's on top
  }
  // Position the proxy (which contains the button) in the center of the
  // viewport
  if (restartButtonProxy) { // Ensure proxy exists before trying to position
    restartButtonProxy->setPos(
        cameraY + viewportCenterX - (restartButton->width() / 2.0),
        cameraY + viewportCenterY + 20);  // Slightly below center
    restartButtonProxy->setVisible(true); // Make proxy visible
  }
}

void Game::restartGame() {
  // Hide game over UI elements
  if (gameOverText) {
    gameOverText->setVisible(false);
  }
  if (restartButtonProxy) {
    restartButtonProxy->setVisible(false);
  }

  // Reset the game to start a new round
  resetGame();
}
