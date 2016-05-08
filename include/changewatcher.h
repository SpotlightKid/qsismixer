#ifndef CHANGEWATCHER_H
#define CHANGEWATCHER_H

#include "qthread.h"
#include "alsa/asoundlib.h"
#include "mixsisctrl.h"
class ChangeWatcher : public QThread{
Q_OBJECT
public:
    ChangeWatcher(snd_ctl_t *ctl, QObject *parent);
    void run();
    snd_ctl_t *ctl;
    void signalChange(int numid, int val, int idx);
signals:
    void changeVal(int numid, int val, int idx);
};

#endif // CHANGEWATCHER_H

