#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "graphics_view.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onOpenProject();

private:
    Tile::GraphicsView* view_;
    QMenu* menu_;
};

#endif // MAINWINDOW_H
