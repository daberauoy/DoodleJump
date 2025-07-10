#include "mainwindow.h"
#include "game.h"
#include "ui_mainwindow.h"
#include <QApplication> // ADD THIS INCLUDE

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      game(nullptr) // Initialize game pointer
{
  ui->setupUi(this);

  // Set up the start menu page
  ui->stackedWidget->setCurrentIndex(0); // Show the start menu page

  // Connect the start button signal to a slot
  connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::startGame);

  // Connect the new close button to the application's quit slot
  connect(ui->closeButton, &QPushButton::clicked, qApp,
          &QApplication::quit); // ADD THIS LINE

  // Set fixed size for the main window to match the game size
  setFixedSize(400, 600);
}

MainWindow::~MainWindow() {
  // Ensure the game object is deleted when the MainWindow is destroyed
  // if it hasn't been already.
  delete game;
  delete ui;
}

void MainWindow::startGame() {
  // Always create a NEW game instance when starting.
  // This ensures a clean state for every game.
  if (game) { // If a previous game instance exists, delete it first.
    ui->stackedWidget->removeWidget(game); // Remove from stacked widget
    delete game;                           // Delete the old game instance
    game = nullptr;                        // Nullify the pointer
  }

  game = new Game(this); // Create a new game instance
  ui->stackedWidget->addWidget(
      game); // Add the new game widget to the stacked widget
  // Connect the game's gameOverSignal to MainWindow's showGameOverMenu slot
  connect(game, &Game::gameOverSignal, this, &MainWindow::showGameOverMenu);

  ui->stackedWidget->setCurrentWidget(game); // Switch to the game page

  // Ensure the game view has focus to receive key events
  game->setFocus();
}

void MainWindow::showGameOverMenu() {
  QMessageBox msgBox(this);
  msgBox.setWindowTitle("Game Over!");
  msgBox.setText("Game Over!\nYour Score: " +
                 QString::number(game->getScore()));
  msgBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Close);
  msgBox.setDefaultButton(QMessageBox::Retry);

  int ret = msgBox.exec();

  if (ret == QMessageBox::Retry) {
    // If user clicks Retry, call restartGame on the existing game instance
    game->restartGame();
  } else {
    // If user clicks Close or closes the dialog, switch back to the start menu
    ui->stackedWidget->setCurrentIndex(0); // Go back to start menu

    // Crucial: Delete the game instance if the user chose to close.
    // This prevents the issue of trying to restart a "dead" game.
    if (game) {
      ui->stackedWidget->removeWidget(game); // Remove from stacked widget
      delete game;                           // Delete the game instance
      game = nullptr; // Set to nullptr to indicate no game is active
    }
  }
}
