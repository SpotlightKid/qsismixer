#include "qthread.h"
#include "mainwindow.h"


class MainWindow::ChangeWatcher : public QThread{

public:
    ChangeWatcher(snd_hctl_t *hctl, QObject *parent = NULL) : QThread(parent), hctl(hctl) {}
    void run()
    {
        snd_hctl_open(&hctl, "hw:USB", 0);
        while(true){
            if(snd_hctl_wait(hctl,10))
            snd_hctl_handle_events(hctl);
        }
    }
    snd_hctl_t *hctl;
};
