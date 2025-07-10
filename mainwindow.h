#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "game.h"
#include <QMainWindow>

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
  void startGame();
  void showGameOverMenu(int finalScore);
  void updateMenuUI(); // NEW: Slot to update the main menu UI state

private:
  Ui::MainWindow *ui;
  Game *game;
  bool gameEndedOnce; // NEW: Flag to track if a game has ended
};
#endif // MAINWINDOW_H
