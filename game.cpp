#include "game.h"
#include <QDebug>
#include <QImage>

Game::Game(QWidget *parent) : QGraphicsView(parent)
{
    // Настройка сцены
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 400, 600);
    setScene(scene);
    setFixedSize(400, 600);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Создание игрока
    QImage playerImage(30, 30, QImage::Format_ARGB32);
    playerImage.fill(Qt::transparent);
    QPainter painter(&playerImage);
    painter.setBrush(Qt::green);
    painter.drawEllipse(0, 0, 30, 30);
    player = new QGraphicsPixmapItem(QPixmap::fromImage(playerImage));
    player->setPos(185, 500);
    scene->addItem(player);

    // Инициализация переменных
    leftPressed = rightPressed = false;
    playerVelocityX = 0;
    playerVelocityY = 0;
    cameraY = 0;
    score = 0;

    // Текст счета
    scoreText = new QGraphicsTextItem();
    scoreText->setPlainText("Score: 0");
    scoreText->setDefaultTextColor(Qt::white);
    scoreText->setPos(10, 10);
    scene->addItem(scoreText);

    // Создание платформ
    createPlatforms();

    // Таймер для обновления игры
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Game::update);
    timer->start(16); // ~60 FPS
}

void Game::createPlatforms()
{
    // Создаем начальные платформы
    for (int i = 0; i < 10; ++i) {
        QGraphicsRectItem *platform = new QGraphicsRectItem(0, 0, 70, 15);
        platform->setBrush(Qt::white);

        int x = QRandomGenerator::global()->bounded(330);
        int y = 600 - i * 60;
        platform->setPos(x, y);

        scene->addItem(platform);
        platforms.append(platform);
    }
    QGraphicsRectItem *platform = new QGraphicsRectItem(0, 0, 70, 15);
    platform->setBrush(Qt::green);

    int x = 165;
    int y = 580;
    platform->setPos(x, y);

    scene->addItem(platform);
    platforms.append(platform);
}

void Game::spawnPlatform()
{
    // Удаляем платформы за пределами экрана
    for (int i = platforms.size() - 1; i >= 0; --i) {
        if (platforms[i]->y() > cameraY + 600) {
            scene->removeItem(platforms[i]);
            delete platforms[i];
            platforms.remove(i);
        }
    }

    // Добавляем новые платформы сверху
    while (platforms.size() < 10) {
        QGraphicsRectItem *platform = new QGraphicsRectItem(0, 0, 70, 15);
        platform->setBrush(Qt::white);

        int x = QRandomGenerator::global()->bounded(330);
        int y = cameraY - QRandomGenerator::global()->bounded(100) - 50;
        platform->setPos(x, y);

        scene->addItem(platform);
        platforms.append(platform);
    }
}

void Game::checkCollisions()
{
    // Проверка столкновений с платформами
    bool onPlatform = false;
    QRectF playerRect = player->mapToScene(player->boundingRect()).boundingRect();

    for (QGraphicsRectItem *platform : platforms) {
        QRectF platformRect = platform->mapToScene(platform->rect()).boundingRect();

        if (playerVelocityY > 0 &&
            playerRect.bottom() <= platformRect.top() + 5 &&
            playerRect.right() > platformRect.left() + 5 &&
            playerRect.left() < platformRect.right() - 5 &&
            playerRect.bottom() + playerVelocityY > platformRect.top())
        {
            playerVelocityY = -15; // Прыжок
            onPlatform = true;

            // Увеличиваем счет при прыжке на новой платформе
            if (platformRect.top() < cameraY) {
                score += 10;
                scoreText->setPlainText("Score: " + QString::number(score));
            }
        }
    }

    // Гравитация
    if (!onPlatform) {
        playerVelocityY += 0.5;
    }
}

void Game::update()
{
    // Управление игроком
    if (leftPressed) playerVelocityX = -5;
    else if (rightPressed) playerVelocityX = 5;
    else playerVelocityX *= 0.9; // Замедление

    // Обновление позиции игрока
    player->setX(player->x() + playerVelocityX);

    // Проверка выхода за границы экрана по горизонтали
    if (player->x() < 0) player->setX(370);
    if (player->x() > 370) player->setX(0);

    // Проверка столкновений
    checkCollisions();

    // Обновление позиции игрока по вертикали
    player->setY(player->y() + playerVelocityY);

    // Камера следует за игроком, когда он поднимается
    if (player->y() < cameraY + 200) {
        int delta = cameraY + 200 - player->y();
        cameraY -= delta;

        // Перемещаем все объекты вниз
        for (QGraphicsItem *item : scene->items()) {
            if (item != player && item != scoreText) {
                item->setY(item->y() + delta);
            }
        }

        // Обновляем позицию игрока (чтобы он оставался на месте относительно экрана)
        player->setY(player->y() + delta);
    }

    // Создаем новые платформы
    spawnPlatform();

    // Проверка на проигрыш
    if (player->y() > cameraY + 600) {
        gameOver();
    }
}

void Game::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left) {
        leftPressed = true;
    } else if (event->key() == Qt::Key_Right) {
        rightPressed = true;
    }
}

void Game::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left) {
        leftPressed = false;
    } else if (event->key() == Qt::Key_Right) {
        rightPressed = false;
    }
}

void Game::gameOver()
{
    timer->stop();

    QGraphicsTextItem *gameOverText = new QGraphicsTextItem();
    gameOverText->setPlainText("Game Over\nScore: " + QString::number(score));
    gameOverText->setDefaultTextColor(Qt::red);
    gameOverText->setFont(QFont("Arial", 24));
    gameOverText->setPos(100, 250);
    scene->addItem(gameOverText);
}
