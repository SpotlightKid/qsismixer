#ifndef CHANGEWATCHER_H
#define CHANGEWATCHER_H

#include "qthread.h"
#include "alsa/asoundlib.h"
#include "mixsisctrl.h"
class ChangeWatcher : public QThread{

public:
    ChangeWatcher(snd_ctl_t *ctl, MixSisCtrl *sisctrl, QObject *parent);
    void run();
    snd_ctl_t *ctl;
    MixSisCtrl *sisctrl;
};

#endif // CHANGEWATCHER_H

