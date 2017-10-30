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
    int loadFrom(QString filen);
    int saveTo(QString filen);
    bool event(QEvent *ev);
    void setChangeWatcherMask(int n, int mask);

public slots:
    void setVal(int alsa_id, int value, int idx = 0);

signals:
    void maskVol(int num, bool mask);

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
    QAction *bounceMtxAct;
    QAction *restoreMtxAct;

    int mtxRAM[18][9];

    void createMenu();
};

#endif // MAINWINDOW_H
