#include "mainwindow.h"
#include "game.h" // Include the Game header
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      game(nullptr) // Initialize game pointer
{
  ui->setupUi(this);

  // Set up the start menu page
  ui->stackedWidget->setCurrentIndex(0); // Show the start menu page

  // Connect the start button signal to a slot
  connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::startGame);

  // Set fixed size for the main window to match the game size
  setFixedSize(400, 600);
}

MainWindow::~MainWindow() {
  // The game object should be deleted when the MainWindow is deleted
  // because it's a child of MainWindow (via 'this' parent in constructor)
  // or explicitly delete it if it's not parented.
  // In this case, `game` is added to stackedWidget, which takes ownership,
  // so `delete game;` here might not be strictly necessary, but it's good
  // practice if `MainWindow` is meant to manage its lifetime.
  delete game; // Clean up the game instance
  delete ui;
}

void MainWindow::startGame() {
  // Create and set up the game instance
  if (!game) {             // Only create if it doesn't exist
    game = new Game(this); // Pass 'this' as parent, so MainWindow owns Game
    ui->stackedWidget->addWidget(
        game); // Add the game widget to the stacked widget
  }
  ui->stackedWidget->setCurrentWidget(game); // Switch to the game page

  // Ensure the game view has focus to receive key events
  game->setFocus();
}
