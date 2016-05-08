#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>

#include "mixsis.h"
#include "changewatcher.h"
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

public slots:
    void setVal(int alsa_id, int value, int idx = 0);

private:
    ChangeWatcher *watch;
    Ui::MainWindow *ui;
    MixSis *mixer;
    MixSisCtrl mixctrl;

    QMenu *fileMenu;
    QAction *saveAct;
    QAction *loadAct;
    QAction *exitAct;
    QMenu *editMenu;
    QAction *clearMtxAct;
    void createMenu();
};

#endif // MAINWINDOW_H
