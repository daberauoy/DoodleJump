#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "game.h" // Include Game header
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
  void startGame(); // Slot to handle starting the game

private:
  Ui::MainWindow *ui;
  Game *game; // Pointer to the Game instance
};
#endif // MAINWINDOW_H
