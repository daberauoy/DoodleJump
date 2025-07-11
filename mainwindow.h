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
  // MODIFIED: startGame now accepts a Difficulty parameter
  void startGame(Difficulty difficulty);
  void showGameOverMenu(int finalScore);
  void updateMenuUI();

private:
  Ui::MainWindow *ui;
  Game *game;
  bool gameEndedOnce;
};
#endif // MAINWINDOW_H
