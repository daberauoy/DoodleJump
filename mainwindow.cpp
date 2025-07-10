#include "mainwindow.h"
#include "game.h"
#include "ui_mainwindow.h"
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), game(nullptr),
      gameEndedOnce(false) // NEW: Initialize flag to false
{
  ui->setupUi(this);

  ui->stackedWidget->setCurrentIndex(0);

  connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::startGame);
  connect(ui->closeButton, &QPushButton::clicked, qApp, &QApplication::quit);

  updateMenuUI(); // NEW: Set initial UI state
  setFixedSize(400, 600);
}

MainWindow::~MainWindow() {
  delete game;
  delete ui;
}

void MainWindow::startGame() {
  if (game) {
    ui->stackedWidget->removeWidget(game);
    delete game;
    game = nullptr;
  }

  game = new Game(this);
  ui->stackedWidget->addWidget(game);
  connect(game, &Game::gameOverSignal, this, &MainWindow::showGameOverMenu);

  ui->stackedWidget->setCurrentWidget(game);

  gameEndedOnce = false; // NEW: Reset flag when a new game starts
  updateMenuUI(); // NEW: Update UI for game in progress (hides score, shows
                  // "Start Game" if not already)

  game->setFocus();
}

void MainWindow::showGameOverMenu(int finalScore) {
  // Update the label on the start menu page with the final score
  ui->scoreDisplayLabel->setText("Game Over!\nYour Score: " +
                                 QString::number(finalScore));

  // Switch back to the start menu page
  ui->stackedWidget->setCurrentIndex(0);

  // Clean up the game instance
  if (game) {
    ui->stackedWidget->removeWidget(game);
    delete game;
    game = nullptr;
  }

  gameEndedOnce = true; // NEW: Set flag to true as a game has now ended
  updateMenuUI(); // NEW: Update UI for game over state (shows score, "Restart
                  // Game" button)
}

// NEW: Implementation of the UI update logic
void MainWindow::updateMenuUI() {
  if (gameEndedOnce) {
    ui->startButton->setText("Restart Game");
    ui->scoreDisplayLabel->setVisible(true);
  } else {
    ui->startButton->setText("Start Game");
    ui->scoreDisplayLabel->setText(""); // Clear score text if no game ended
    ui->scoreDisplayLabel->setVisible(false); // Hide score label
  }
}
