#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>

#include "mixsis.h"

extern const char* device;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void loop();
    int loadFrom(const char* filename);
    int saveTo(const char* filename);
    bool event(QEvent *ev);

private:
        class ChangeWatcher;
        ChangeWatcher * cw;
    Ui::MainWindow *ui;
    MixSis *mixer;
    MixSisCtrl mixctrl;

    QMenu *fileMenu;
    QAction *saveAct;
    QAction *loadAct;
    QAction *exitAct;
    void createMenu();
};

#endif // MAINWINDOW_H
