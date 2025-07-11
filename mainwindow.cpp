#include "mainwindow.h"
#include "game.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QDebug> // For debugging

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), game(nullptr),
      gameEndedOnce(false) {
  ui->setupUi(this);

  // Hide the status bar to prevent it from appearing as a "line" at the bottom
  if (ui->statusbar) {
    ui->statusbar->hide();
  }

  ui->stackedWidget->setCurrentIndex(0);

  // === NEW: Set background for startMenuPage using stylesheet ===
  ui->startMenuPage->setStyleSheet(
      "QWidget#startMenuPage { background-image: "
      "url(:/sprites/background_menu.png);"
      "background-position: center;"
      "background-repeat: no-repeat;"
      "background-size: cover; }" // Use cover to make it fill the space
  );
  // =============================================================

  // Connect startButton to startGame, reading difficulty from ComboBox
  connect(ui->startButton, &QPushButton::clicked, this, [this]() {
    // Get selected difficulty from QComboBox
    Difficulty selectedDifficulty =
        static_cast<Difficulty>(ui->difficultyComboBox->currentIndex());
    startGame(selectedDifficulty);
  });
  connect(ui->closeButton, &QPushButton::clicked, qApp, &QApplication::quit);

  updateMenuUI();
  setFixedSize(400, 600);
}

MainWindow::~MainWindow() {
  delete game;
  delete ui;
}

// MODIFIED: startGame now accepts a Difficulty parameter
void MainWindow::startGame(Difficulty difficulty) {
  if (game) {
    ui->stackedWidget->removeWidget(game);
    delete game;
    game = nullptr;
  }

  // Pass the selected difficulty to the Game constructor
  game = new Game(difficulty, this);
  ui->stackedWidget->addWidget(game);
  connect(game, &Game::gameOverSignal, this, &MainWindow::showGameOverMenu);

  ui->stackedWidget->setCurrentWidget(game);

  gameEndedOnce = false;
  updateMenuUI();

  game->setFocus();
}

void MainWindow::showGameOverMenu(int finalScore) {
  ui->scoreDisplayLabel->setText("Game Over!\nYour Score: " +
                                 QString::number(finalScore));

  ui->stackedWidget->setCurrentIndex(0);

  if (game) {
    ui->stackedWidget->removeWidget(game);
    delete game;
    game = nullptr;
  }

  gameEndedOnce = true;
  updateMenuUI();
}

void MainWindow::updateMenuUI() {
  if (gameEndedOnce) {
    ui->startButton->setText("Restart Game");
    ui->scoreDisplayLabel->setVisible(true);
    ui->difficultyLabel->setVisible(
        true); // Show difficulty selection on restart
    ui->difficultyComboBox->setVisible(true);
  } else {
    ui->startButton->setText("Start Game"); // Changed from "" to "Start Game"
    ui->scoreDisplayLabel->setText("");
    ui->scoreDisplayLabel->setVisible(false);
    ui->difficultyLabel->setVisible(
        true); // Always show difficulty selection when starting fresh
    ui->difficultyComboBox->setVisible(true);
  }
}
