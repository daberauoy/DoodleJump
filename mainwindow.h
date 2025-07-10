#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "game.h" // Include Game header
#include <QMainWindow>
#include <QMessageBox> // Include QMessageBox

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void startGame();        // Slot to handle starting the game
  void showGameOverMenu(); // New slot to show the game over dialog

private:
  Ui::MainWindow *ui;
  Game *game; // Pointer to the Game instance
};
#endif // MAINWINDOW_H
